#!/bin/sh

va_line=$(netstat -nap | grep "yymms" | awk '{print $7}')
va_line=$(echo $va_line | awk -F"/" '{print $1}')

if [ -z "$va_line" ]; then
	echo "all dead"
else
	echo "pid=" $va_line " kill it"
	kill -s 10 $va_line
fi


