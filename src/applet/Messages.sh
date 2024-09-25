#!/usr/bin/env bash

$XGETTEXT `find . -name '*.js' -o -name '*.cpp' -o -name '*.qml'` -o $podir/plasma_applet_org.kde.plasma.bluetooth.pot
