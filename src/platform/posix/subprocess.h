#ifndef POSIX_SUBPROCESS_H
#define POSIX_SUBPROCESS_H

#include "../subprocess.h"
#include <unistd.h>

class PosixSubprocess : public Subprocess {
public:
	PosixSubprocess();
	virtual ~PosixSubprocess() override;
	virtual ProcessError start(const char *executeable, char *const *args) override;
	virtual ProcessError poll() override;
	virtual ProcessError write_message(const void *buffer, size_t n) override;
	virtual ProcessError kill_process(int signal = 9) override;
	virtual char *read_buffer() override;
	virtual char *read_error_buffer() override;

private:
	int stdio_pipe[2];
	int stderr_pipe[2];
	pid_t proccess_pid = 0;

	char buffer[SUBPROCESS_BUFFER_SIZE];
	int current_error_buffer_size = 0;

	char error_buffer[SUBPROCESS_BUFFER_SIZE];
	int current_buffer_size = 0;
};

#endif