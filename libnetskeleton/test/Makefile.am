INCLUDES = -I../include
sbin_PROGRAMS = test 
test_SOURCES = test.c
test_CFLAGS = -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE
test_LDFLAGS = -lrt -ldl -levent
test_LDADD = ../src/.libs/libnetskeleton.la 
