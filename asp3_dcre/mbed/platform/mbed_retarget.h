#ifndef RETARGET_H
#define RETARGET_H

#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif //__cplusplus

#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>

#ifdef __cplusplus
namespace mbed {

class FileHandle;
class DirHandle;

std::FILE *fdopen(FileHandle *fh, const char *mode);

}
#endif

#endif //RETARGET_H
