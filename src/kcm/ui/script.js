function deviceTypeToString(type) {
    switch (type) {
    case BluezQt.Device.Phone:
        return i18nc("This device is a Phone", "Phone");
    case BluezQt.Device.Modem:
        return i18nc("This device is a Modem", "Modem");
    case BluezQt.Device.Computer:
        return i18nc("This device is a Computer", "Computer");
    case BluezQt.Device.Network:
        return i18nc("This device is of type Network", "Network");
    case BluezQt.Device.Headset:
        return i18nc("This device is a Headset", "Headset");
    case BluezQt.Device.Headphones:
        return i18nc("This device is a Headphones", "Headphones");
    case BluezQt.Device.AudioVideo:
        return i18nc("This device is an Audio/Video device", "Multimedia");
    case BluezQt.Device.Keyboard:
        return i18nc("This device is a Keyboard", "Keyboard");
    case BluezQt.Device.Mouse:
        return i18nc("This device is a Mouse", "Mouse");
    case BluezQt.Device.Joypad:
        return i18nc("This device is a Joypad", "Joypad");
    case BluezQt.Device.Tablet:
        return i18nc("This device is a Graphics Tablet (input device)", "Tablet");
    case BluezQt.Device.Peripheral:
        return i18nc("This device is a Peripheral device", "Peripheral");
    case BluezQt.Device.Camera:
        return i18nc("This device is a Camera", "Camera");
    case BluezQt.Device.Printer:
        return i18nc("This device is a Printer", "Printer");
    case BluezQt.Device.Imaging:
        return i18nc("This device is an Imaging device (printer, scanner, camera, display, …)", "Imaging");
    case BluezQt.Device.Wearable:
        return i18nc("This device is a Wearable", "Wearable");
    case BluezQt.Device.Toy:
        return i18nc("This device is a Toy", "Toy");
    case BluezQt.Device.Health:
        return i18nc("This device is a Health device", "Health");
    default:
        const profiles = [];

        if (Uuids.indexOf(BluezQt.Services.ObexFileTransfer) !== -1) {
            profiles.push(i18n("File transfer"));
        }
        if (Uuids.indexOf(BluezQt.Services.ObexObjectPush) !== -1) {
            profiles.push(i18n("Send file"));
        }
        if (Uuids.indexOf(BluezQt.Services.HumanInterfaceDevice) !== -1) {
            profiles.push(i18n("Input"));
        }
        if (Uuids.indexOf(BluezQt.Services.AdvancedAudioDistribution) !== -1) {
            profiles.push(i18n("Audio"));
        }
        if (Uuids.indexOf(BluezQt.Services.Nap) !== -1) {
            profiles.push(i18n("Network"));
        }

        if (!profiles.length) {
            profiles.push(i18n("Other"));
        }

        labels.push(profiles.join(", "));
    }
}
