#!/bin/bash

killall qvfb
qvfb -width 320 -height 240 -depth 16 -zoom 1 -qwsdisplay 0 -skin mini2440.skin &
sleep 1
