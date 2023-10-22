#include "subprocess.h"

#if defined(LCE_PLATFORM_POSIX)
#include "posix/subprocess.h"
#elif defined(LCE_PLATFORM_WINDOWS)
#include "windows/subprocess.h"
#endif

bool Subprocess::is_running() const {
	return running;
}

std::string Subprocess::error_to_string(Error err) {
	switch (err) {
		case OK:
			return "No error";
		case NOT_RUNNING:
			return "Process is not running";
		case ALREAD_RUNNING:
			return "Process is already running";
		case NOTHING_TO_READ:
			return "Nothing to read";
		case FAILED_TO_KILL:
			return "Failed to kill process";
		case FAILED_TO_WRITE:
			return "Failed to write to process";
		case FAILED_TO_READ:
			return "Failed to read from process";
		case FAILED_TO_CREATE_PIPE:
			return "Failed to create pipe";
		default:
			return "Unknown error";
	}
}

Subprocess *Subprocess::new_platform_subprocess() {
#if defined(LCE_PLATFORM_POSIX)
	return new PosixSubprocess();
#elif defined(LCE_PLATFORM_WINDOWS)
	return new WindowsSubprocess();
#endif
}