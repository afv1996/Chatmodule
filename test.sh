#!/bin/bash
while :
do
	read CHECK < /proc/3143/fd/2
	echo "$CHECK"
	if [ $CHECK != 0 ]
	then
		break
	else
		echo "waiting....."
	fi
done
