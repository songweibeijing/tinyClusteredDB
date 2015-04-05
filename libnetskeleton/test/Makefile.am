INCLUDES = -I../include
sbin_PROGRAMS = test 
test_SOURCES = test.c
test_CFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE
test_LDFLAGS = -ldl -L/usr/local/lib -levent
test_LDADD = ../src/libnetskeleton.la 
