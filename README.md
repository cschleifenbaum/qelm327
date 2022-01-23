# qelm327
QCanBus plugin for ELM327

# How to use this plugin

Use QCanBus::createDevice to create your device:

`QCanBusDevice* dev = QCanBus::instance()->createDevice("elm327can", "/dev/tty.OBDII-SPPDev");`

The second parameter is the device name of the connected Bluetooth ELM327.

## Device frame ID filter

To be able to filter messages by their frame ID (cf. ELM327 `ATCM` command), you can use the Qt property `frameIdFilter`. It accepts an `int`.
