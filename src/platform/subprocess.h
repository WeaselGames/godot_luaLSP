#ifndef SUBPROCESS_H
#define SUBPROCESS_H

#define SUBPROCESS_BUFFER_SIZE 1024

#include <stddef.h>

class Subprocess {
protected:
	bool running = false;

public:
	enum ProcessError {
		OK,
		NOT_RUNNING,
		ALREAD_RUNNING,
		BUFFER_FULL,
		NOTHING_TO_READ,
		FAILED_TO_KILL,
		FAILED_TO_WRITE,
		FAILED_TO_READ,
		FAILED_TO_CREATE_PIPE,
	};

	virtual ~Subprocess() = default;
	virtual ProcessError start(const char *executeable, char *const *args) = 0;
	virtual ProcessError write_message(const void *buffer, size_t n) = 0;
	virtual ProcessError poll() = 0;
	virtual ProcessError kill_process(int signal = 9) = 0;
	virtual char *read_buffer() = 0;
	virtual char *read_error_buffer() = 0;

	bool is_running() const;

	static Subprocess *new_platform_subprocess();
};

#endif