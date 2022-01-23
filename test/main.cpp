#include <QCoreApplication>
#include <QCanBus>
#include <QCanBusFrame>
#include <QDebug>
#include <QLoggingCategory>
#include <QTimer>

class Frame : public QCanBusFrame
{
public:
    Frame(quint32 id, const QVector<quint8>& d)
        : QCanBusFrame(id, QByteArray((char*)d.data(), d.size()))
    {
    }
};

double readField(const QCanBusFrame& f, int bitpos, int length, double factor = 1.0, int offset = 0)
{
    int result = 0;

    int bitsread = 0;

    const QByteArray data = f.payload();
    while (length > 0) {
        int byte = bitpos / 8;
        int localbitpos = bitpos % 8;
        int bitstoread = std::min(length, 8 - localbitpos);
        uchar value = data[byte];
        uchar mask = uchar(0xff << 8 >> bitstoread) >> (8 - bitstoread) << localbitpos;
        int shift = localbitpos - bitsread;
        result += (int(value & mask) >> std::max(0, shift) << std::max(0, -shift));

        bitsread += bitstoread;
        length -= bitstoread;
        bitpos += bitstoread - 16;
    }

    return result * factor + offset;
}

#define setValue(VAR, VALUE) \
{ \
    auto v = VALUE; \
    if (v != VAR) { \
        VAR = VALUE; \
        qDebug() << #VAR << v; \
    } \
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    QLoggingCategory::setFilterRules(QStringLiteral("qt.canbus* = true"));

    QCanBusDevice* dev = QCanBus::instance()->createDevice("elm327can", "/dev/tty.OBDII-SPPDev");

    double soc = 0.0;
    double current = 0.0;
    double voltage = 0.0;
    double maximumObChargerPower = 0.0;
    double batteryMaximumPower = -1.0;
    double batteryMaximumCharge = -1.0;

    QObject::connect(dev, &QCanBusDevice::framesReceived, [&]() {
        auto frames = dev->readAllFrames();
        for (const auto& f : frames) {
            switch (f.frameId()) {
            case 0x1dc:
                setValue(batteryMaximumPower, readField(f, 14, 10, 0.25));
//                setValue(batteryMaximumCharge, readField(f, 20, 10, 0.25));
                setValue(batteryMaximumCharge, readField(f, 26, 10, 0.1, -10));
                break;
            case 0x390:
                setValue(maximumObChargerPower, readField(f, 48, 9, 0.1));
                break;
            case 0x55b:
                setValue(soc, readField(f, 14, 10, 0.1));
                break;
            case 0x1db:
                setValue(current, readField(f, 13, 11, 0.5));
                setValue(voltage, readField(f, 30, 10, 0.5));
                break;
            }
        }
    });

    QVector<int> frameIdFilters{0x1dc, 0x390, 0x55b, 0x1db};
    int currentId = 0;
    auto frameIdFilterTimer = new QTimer(dev);
    QObject::connect(frameIdFilterTimer, &QTimer::timeout, [&]() {
        currentId = (currentId + 1) % frameIdFilters.count();
        dev->setProperty("frameIdFilter", frameIdFilters.at(currentId));
    });

    dev->setProperty("frameIdFilter", frameIdFilters.at(currentId));

    if (!dev->connectDevice()) {
        qDebug() << dev->errorString();
        return 0;
    }
    frameIdFilterTimer->start(5000);

    return app.exec();
}
