#pragma once

#include <QCanBusDevice>
#include <QSerialPort>

class Elm327CanBusDevice : public QCanBusDevice
{
    Q_OBJECT
    Q_PROPERTY(int frameIdFilter WRITE setFrameIdFilter READ frameIdFilter)
public:
    Elm327CanBusDevice(const QString& port, QObject* parent = nullptr);
    ~Elm327CanBusDevice();

    bool writeFrame(const QCanBusFrame& frame) override;
    
    QString interpretErrorFrame(const QCanBusFrame& frame) override;

    void setFrameIdFilter(int frameIdFilter);
    int frameIdFilter() const;

protected:
    bool open() override;
    void close() override;

    void readData();
    QString send(const QByteArray& data, int timeout = 100);

private:
    int m_frameIdFilter = 0;
    QSerialPort m_port;
    QByteArray m_buffer;
};
