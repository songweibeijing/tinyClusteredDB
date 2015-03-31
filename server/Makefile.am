INCLUDES = -I../include
sbin_PROGRAMS = clustered_server
clustered_server_SOURCES = server.cpp
clustered_server_CFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE
clustered_server_CPPFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE -std=gnu++0x
clustered_server_LDFLAGS = -lpthread -lrt -ldl -lprotobuf
clustered_server_LDADD = ../common/libcommon.la \
			 ../libnetskeleton/src/.libs/libnetskeleton.la 
