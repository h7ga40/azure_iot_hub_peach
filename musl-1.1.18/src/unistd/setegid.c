#include <unistd.h>
#include "libc.h"
#include "syscall.h"

int setegid(gid_t egid)
{
	return __setxid(__NR_setresgid, -1, egid, -1);
}
