#!/bin/sh
INPUT_FILE=$1
SQLITE_FILE=$2
STATE_ACCESS="OFF"
CURRENT_CLASS=""
VERBOSE="ON"

sqlite_query () {
	if [ "${VERBOSE}" = "ON" ]
	then
		echo "query ... $1"
	fi
	sqlite3 $SQLITE_FILE "$1"
}

check_name () {
	if [ "${STATE_ACCESS}" = "ON" ]
	then
		#echo "    $1"
		sqlite_query "INSERT INTO ACCESS_VECTORS VALUES(\"$1\");"
		sqlite_query "INSERT INTO ATOC VALUES(\"$CURRENT_CLASS\",\"$1\");"
		return 0;
	else
		echo "Invalid state"
		exit
	fi
}

check_access_start () {
	if [ "${STATE_ACCESS}" = "ON" ]
	then
		echo "Error! Duplicate {."
		exit
	fi
	STATE_ACCESS="ON"
}

check_access_finish () {
	if [ "${STATE_ACCESS}" = "OFF" ]
	then
		echo "Error! Missing }."
		exit
	fi
	STATE_ACCESS="OFF"
}

check_type () {
	NAME=$1
	case $1 in
	*common*)
		NAME=${NAME##common }
		#echo "common $NAME"
		sqlite_query "INSERT INTO CLASS VALUES(\"common-$NAME\");" 
		CURRENT_CLASS="common-$NAME"
		;;
	*class*)
		NAME=${NAME##class }
		#echo "class $NAME";;
		CURRENT_CLASS=$NAME
		sqlite_query "INSERT INTO CLASS VALUES(\"$NAME\");"
		;;
	*inherits*)
		NAME=${NAME##inherits }
		#echo "inherit $NAME"
		AVECTORS=`sqlite3 $SQLITE_FILE "SELECT aname from ATOC where cname=\"common-$NAME\";"` 
		for AVECTOR in $AVECTORS; 
		do
			echo "Inheritance--"
			sqlite_query "INSERT INTO ATOC VALUES(\"$CURRENT_CLASS\",\"$AVECTOR\");"
		done
		;;
	{)
		check_access_start 
		#echo "{"
		;;
	})
		check_access_finish 
		#echo "}"
		;;
	*)
		check_name $1
		;;
	esac
}

sqlite_query "BEGIN;"
sqlite_query "CREATE TABLE CLASS(cname TEXT PRIMARY KEY);"
sqlite_query "CREATE TABLE ACCESS_VECTORS(aname TEXT PRIMARY KEY);"
sqlite_query "CREATE TABLE ATOC(cname TEXT, aname TEXT);"
cat $INPUT_FILE | while read LINE
do
	# to eliminate comment line.
	LINE_NO_COMMENT=${LINE%%#*}
	if [ "${LINE_NO_COMMENT}" != "" ]
	then
		#
		# シェル関数にスペースを含む文字列を"なしで渡すと
		# 引数2つと解釈されるので注意
		#
		check_type "$LINE_NO_COMMENT"
	fi
done
sqlite_query "COMMIT;"
