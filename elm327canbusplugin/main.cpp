#include <QObject>
#include <QCanBusFactoryV2>

#include "elm327canbusdevice.h"

#include <QSerialPortInfo>

#include <QDebug>
class Elm327CanbusPlugin : public QObject, public QCanBusFactoryV2
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QCanBusFactory" FILE "plugin.json")
    Q_INTERFACES(QCanBusFactoryV2)

public:
    QList<QCanBusDeviceInfo> availableDevices(QString *errorMessage) const override
    {
        qDebug() << "available";
        QList<QCanBusDeviceInfo> devices;
        for (const auto& port : QSerialPortInfo::availablePorts()) {
        }
        return devices;
    }

    QCanBusDevice *createDevice(const QString &interfaceName, QString *errorMessage) const override
    {
        Q_UNUSED(errorMessage)
        return new Elm327CanBusDevice(interfaceName);
    }
};

#include "main.moc"
