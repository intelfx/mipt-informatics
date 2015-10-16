#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#define _cleanup_(x) \
	__attribute__((cleanup(x)))
#define _unused_ \
	__attribute__((unused))

#define DEFINE_TRIVIAL_CLEANUP_FUNC(type, func) \
        static inline void func##p(type *p) {                   \
		if (*p)                                         \
			func(*p);                               \
	}                                                       \
        struct __useless_struct_to_allow_trailing_semicolon__

DEFINE_TRIVIAL_CLEANUP_FUNC(void*, free);
#define _cleanup_free_ _cleanup_(freep)

void safe_fclose(FILE* file)
{
	if (file != NULL)
		fclose(file);
}
DEFINE_TRIVIAL_CLEANUP_FUNC(FILE*, safe_fclose);
#define _cleanup_fclose_ _cleanup_(safe_fclosep)

void safe_close(int fd)
{
	if (fd >= 0)
		close(fd);

}
DEFINE_TRIVIAL_CLEANUP_FUNC(int, safe_closep);
#define _cleanup_close_ _cleanup_(safe_closep)

void errnop(int *saved_errno)
{
	if (saved_errno != NULL)
		errno = *saved_errno;
}
#define _cleanup_errno_ _cleanup_(errnop)
#define PRESERVE_UMASK \
	_cleanup_errno_ _unused_ int _saved_errno_ = errno

int main(int argc, char **argv)
{
}
