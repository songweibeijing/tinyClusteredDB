INCLUDES = -I../include -I/include
lib_LTLIBRARIES=libcommon.la
libcommon_la_SOURCES=md5.cpp  util.cpp conf.cpp message.pb.cc message.cpp
libcommon_la_CPPFLAGS=-D_FILE_OFFSET_BITS=64 -D_LARGE_FILE -std=gnu++0x -g
libcommon_la_LDFLAGS=-lpthread -lrt -ldl -lprotobuf
