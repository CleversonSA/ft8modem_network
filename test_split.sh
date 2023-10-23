#!/bin/bash
#
#  Script to test split operation on a real transceiver
#

sleep 10
f=200
while [ $f -lt 3000 ]; do # [ 1 ] ; do 
	echo "$f KK5JY AT $f"
	f=$(( $f + $(( $RANDOM / 100 )) ))
	sleep 30
done

# EOF
