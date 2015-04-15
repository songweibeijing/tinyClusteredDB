#!/bin/sh

while [ 1 ]
do
	sh ./testclient &
	sleep 2s
	killall lt-testclient 
done
