/* Wrapper to implement ANSI C's atexit using SunOS's on_exit. */
/* This function is in the public domain.  --Mike Stump. */

#include <config_libgnu.h>
#if !defined(TARGET_OS_ANDROID) && !defined(TARGET_OS_WINDOWS)

static void atexit_adapter(int status, void *arg)
{
    (void)status;
    void (*f)(void) = arg;
    f();
}

int atexit (void (*f) (void))
{
  /* If the system doesn't provide a definition for atexit, use on_exit
     if the system provides that.  */
    return on_exit(atexit_adapter, f);
}

#endif //  !defined(TARGET_OS_ANDROID) && !defined(TARGET_OS_WINDOWS)
