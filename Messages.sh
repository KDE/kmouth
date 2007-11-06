#! /usr/bin/env bash
$EXTRACTRC *.rc *.ui */*.ui >> ./rc.cpp || exit 11
$XGETTEXT `find . -name \*.h -o -name \*.cpp` -o $podir/kmouth.pot

