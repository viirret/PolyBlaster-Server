#!/bin/bash

screen -list | grep -q polyblaster

if [[ $? -eq 0 ]]; then
	echo 'Server is already running!'
else
	echo 'Starting server in screen!'
	screen -dmS polyblaster $HOME/PolyBlaster/server
	echo 'Open console: screen -r polyblaster'
fi

