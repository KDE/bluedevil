#!/usr/bin/env bash

$EXTRACTRC `find . -name \*.ui -o -name \*.rc -o -name \*.kcfg` >> rc.cpp
$XGETTEXT `find . \( -name \*.cpp -o -name \*.qml \) ! -path "*applet*" ! -path "*kcm*"` -o $podir/bluedevil.pot
