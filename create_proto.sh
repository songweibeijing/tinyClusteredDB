#!/bin/sh

PROTOC=`which protoc` 
if [ $? -ne 0 ];then
	echo ""
	echo "no protoc can be found, please install it"
	exit -1;
fi

protoc --cpp_out=./ ./message.proto
mv ./message.pb.h ./include/
mv ./message.pb.cc ./common/
