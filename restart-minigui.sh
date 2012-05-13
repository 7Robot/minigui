#!/bin/sh

killall -KILL minigui

cd /home
ipkg install minigui_1.0.0_armel.ipk

. /etc/profile.environment

nohup minigui -qws > /dev/null < /dev/null &
