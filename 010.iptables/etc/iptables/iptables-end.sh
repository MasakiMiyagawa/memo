#!/bin/bash 

SCRIPT_DIR=`dirname $0`
ERROR=0

echo "Try latest setting ..."
/usr/sbin/iptables-save > ${SCRIPT_DIR}/ip4tables.rules.save
ERROR=$?

if [ ${ERROR} -eq 0 ]
then
	echo "Success."
else
	echo "Failure(${ERROR})"
	exit ${ERROR}
fi
