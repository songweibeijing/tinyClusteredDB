INCLUDES = -I../include -I../libnetskeleton/include -I/include
sbin_PROGRAMS = clustered_server test
clustered_server_SOURCES = server.cpp
clustered_server_CFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE
clustered_server_CPPFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE -std=gnu++0x -g
clustered_server_LDFLAGS = -lpthread -lrt -ldl -lprotobuf -L/usr/local/lib -levent
clustered_server_LDADD = ../common/libcommon.la \
			 ../libnetskeleton/src/.libs/libnetskeleton.la 
test_SOURCES = test.cpp
test_CFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE
test_CPPFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE -std=gnu++0x -g
test_LDFLAGS = -lpthread -lrt -ldl -lprotobuf -L/usr/local/lib -levent
test_LDADD = ../common/libcommon.la \
	     ../libnetskeleton/src/.libs/libnetskeleton.la
