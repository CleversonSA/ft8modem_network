#!/bin/bash

SOUNDCARD_DEV_ID=`cat /proc/asound/cards | grep "USB" | head -n 1 | awk -F ' ' '{ print $1 }'`

if [ "${SOUNDCARD_DEV_ID}" == "" ]; then
	
	clear
	echo "FATAL ERROR: NO USB SOUNDCARD FOUND! PLEASE INSERT ONE AND TRY AGAIN!"
  /bin/sleep 5
  exit -1

fi

./ft8modem ft8 ${SOUNDCARD_DEV_ID}
exit 0

