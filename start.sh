#!/bin/bash

screen -list | grep -q polyblaster$1

if [[ $? -eq 0 ]]; then
	echo 'Server is already running!'
else
	echo 'Starting server in screen!'
	screen -dmS polyblaster$1 $HOME/PolyBlaster/server "$@"
	echo "Open console: screen -r polyblaster$1"
fi

