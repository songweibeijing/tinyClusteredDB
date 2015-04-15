INCLUDES = -I../include
sbin_PROGRAMS = testserver testclient server client
testserver_SOURCES = test_server.c
testserver_CFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE
testserver_LDFLAGS = -ldl -L/usr/local/lib -levent
testserver_LDADD = ../src/libnetskeleton.la

testclient_SOURCES = test_client.c
testclient_CFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE
testclient_LDFLAGS = -ldl -L/usr/local/lib -levent -lpthread
testclient_LDADD = ../src/libnetskeleton.la

client_SOURCES = client.c
client_CFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE
client_LDFLAGS = -ldl -L/usr/local/lib -levent -lpthread

server_SOURCES = server.c
server_CFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE
server_LDFLAGS = -ldl -L/usr/local/lib -levent -lpthread
