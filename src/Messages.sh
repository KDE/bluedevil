#!/usr/bin/env bash

$EXTRACTRC `find . -name \*.ui -o -name \*.rc -o -name \*.kcfg` >> rc.cpp
$XGETTEXT `find . -name \*.cpp | grep -v '/obextest/'` -o $podir/bluedevil.pot
