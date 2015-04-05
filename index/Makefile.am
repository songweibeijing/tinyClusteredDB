INCLUDES = -I../include
sbin_PROGRAMS = clustered_index
clustered_index_SOURCES = index.cpp
clustered_index_CFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE
clustered_index_CPPFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE -std=gnu++0x -g
clustered_index_LDFLAGS = -lpthread -lrt -ldl -lprotobuf
clustered_index_LDADD = ../common/libcommon.la 
