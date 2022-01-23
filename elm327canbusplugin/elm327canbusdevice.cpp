#include "elm327canbusdevice.h"

Elm327CanBusDevice::Elm327CanBusDevice(const QString& port, QObject* parent)
    : QCanBusDevice(parent),
      m_port(port)
{
}
    
Elm327CanBusDevice::~Elm327CanBusDevice()
{
}

bool Elm327CanBusDevice::writeFrame(const QCanBusFrame& frame)
{
    send(QString("AT SH %1\r").arg(frame.frameId(), 0, 16).toLocal8Bit());
    QStringList payloadStrings;
    const QByteArray payload = frame.payload();
    std::transform(payload.begin(), payload.end(), std::back_inserter(payloadStrings), [](char c) { return QString("%1").arg((uint)c, 0, 16); });
    send(payloadStrings.join(" ").toLocal8Bit());
    return true;
}
    
QString Elm327CanBusDevice::interpretErrorFrame(const QCanBusFrame& frame)
{
    Q_UNUSED(frame)
    // we don't know about the actual contents...
    return {};
}

void Elm327CanBusDevice::setFrameIdFilter(int frameIdFilter)
{
    if (m_frameIdFilter == frameIdFilter)
        return;
    m_frameIdFilter = frameIdFilter;
    if (state() == ConnectedState) {
        disconnect(&m_port, &QSerialPort::readyRead, this, &Elm327CanBusDevice::readData);
        send("\r");
        if (m_frameIdFilter) {
            send(QStringLiteral("ATCF %1\r").arg(m_frameIdFilter, 0, 16).toLocal8Bit());
            send("ATCM 7FF\r");
        } else {
            send("ATCF 000\r");
            send("ATCM 000\r");
        }
        connect(&m_port, &QSerialPort::readyRead, this, &Elm327CanBusDevice::readData);
        send("ATMA\r");
    }
}

int Elm327CanBusDevice::frameIdFilter() const
{
    return m_frameIdFilter;
}

bool Elm327CanBusDevice::open()
{
    setState(ConnectingState);
    if (!m_port.open(QIODevice::ReadWrite)) {
        setError(m_port.errorString(), QCanBusDevice::ConnectionError);
        setState(UnconnectedState);
        return false;
    }
    m_port.setBaudRate(115200);

    send("ATZ\r", 1000);
    send("ATH1\r");
    send("ATCAF0\r");
    send("ATDP\r");
    if (m_frameIdFilter) {
        send(QString("ATCF %1\r").arg(m_frameIdFilter, 0, 16).toLocal8Bit());
        send("ATCM 7FF\r");
    } else {
        send("ATCF 000\r");
        send("ATCM 000\r");
    }
    connect(&m_port, &QSerialPort::readyRead, this, &Elm327CanBusDevice::readData);
    setState(ConnectedState);
    m_port.write("ATMA\r");
    return true;
}

void Elm327CanBusDevice::close()
{
    setState(ClosingState);
    m_port.close();
    setState(UnconnectedState);
}

void Elm327CanBusDevice::readData()
{
    auto data = m_port.readAll();
    m_buffer += data;

    int index = m_buffer.indexOf('\r');
    QVector<QCanBusFrame> frames;
    while (index != -1) {
        auto line = QString::fromLocal8Bit(m_buffer.left(index));
        if (line == "BUFFER FULL")
            setError(line, UnknownError);
        m_buffer.remove(0, index + 1);
        auto parts = line.split(" ", QString::SkipEmptyParts);
        if (parts.count() > 1) {
            bool ok = false;
            const auto idString = parts.takeFirst();
            quint32 id = idString.toUInt(&ok, 16);
            if (!ok || idString.length() != 3)
                return;
            QByteArray payload;
            if (parts.count() != 8)
                return;
            for (const auto& part : parts) {
                uchar v = part.toUInt(&ok, 16);
                if (!ok || part.length() != 2)
                    return;
                payload.append(v);
            }
            frames.push_back({QCanBusFrame(id, payload)});
        }
        index = m_buffer.indexOf('\r');
    }
    if (!frames.isEmpty()) {
        enqueueReceivedFrames(frames);
    }
}
    
QString Elm327CanBusDevice::send(const QByteArray& data, int timeout)
{
	QString buffer;
    m_port.write(data);

    while (m_port.waitForReadyRead(timeout)) {
        buffer += m_port.readAll();
    }

    //removes the request from response
    buffer = buffer.mid(data.length());
    return buffer; 
}
