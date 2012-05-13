#!/bin/sh

ipkg install minigui_1.0.0_armel.ipk
killall minigui

nohup minigui -qws > /dev/null &
