#!/bin/bash 

SCRIPT_DIR=`dirname $0`
ERROR=0

echo "Try latest setting ..."
/usr/sbin/iptables-restore < ${SCRIPT_DIR}/ip4tables.rules.save
ERROR=$?

if [ ${ERROR} -ne 0 ]
then
	echo "Failure(${ERROR}) Try original setting ..."
	/usr/sbin/iptables-restore < ${SCRIPT_DIR}/ip4tables.rules.org
	ERROR=$?
	if [ ${ERROR} -eq 0 ]
	then
		echo "Success."
	else
		echo "Failure(${ERROR})."
		exit ${ERROR}
	fi
else
	echo "Success."
fi
