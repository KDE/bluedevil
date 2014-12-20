#ifndef FILEITEMACTIONPLUGINTYPES
#define FILEITEMACTIONPLUGINTYPES

#include <QMetaType>

typedef QMap<QString, QString> DeviceInfo;
typedef QMap<QString, DeviceInfo> QMapDeviceInfo;
Q_DECLARE_METATYPE(DeviceInfo)
Q_DECLARE_METATYPE(QMapDeviceInfo)

#endif
