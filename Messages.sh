#! /usr/bin/env bash
$EXTRACTRC *.rc *.ui */*.ui >> ./rc.cpp || exit 11
$XGETTEXT *.cpp -o $podir/kmouth.pot

