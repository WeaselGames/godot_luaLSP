#include "subprocess.h"

#if defined(LCE_PLATFORM_POSIX)
#include "posix/subprocess.h"
#elif defined(LCE_PLATFORM_WINDOWS)
#include "windows/subprocess.h"
#endif

Subprocess *Subprocess::new_platform_subprocess() {
#if defined(LCE_PLATFORM_POSIX)
	return new PosixSubprocess();
#elif defined(LCE_PLATFORM_WINDOWS)
	return new WindowsSubprocess();
#endif
}

bool Subprocess::is_running() const {
	return running;
}