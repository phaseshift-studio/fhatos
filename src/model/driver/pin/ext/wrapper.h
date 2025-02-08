#pragma once
#ifndef WRAPPER_h
#define WRAPPER_h
#ifdef NATIVE
#include <inttypes.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
ssize_t wrappedWrite(int fd, void *buf, size_t count)
{
	write(fd,buf,count);
}

ssize_t wrappedRead(int fd, void *buf, size_t count)
{
	read(fd,buf,count);
}
#endif
#endif
