#!/usr/bin/env bash

$XGETTEXT `find . -name '*.cpp' -o -name '*.qml' -o -name '*.js'` -o $podir/kcm_bluetooth.pot
