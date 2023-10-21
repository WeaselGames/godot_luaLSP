#ifndef WINDOWS_SUBPROCESS_H
#define WINDOWS_SUBPROCESS_H

#include "../subprocess.h"

class WindowsSubprocess : public Subprocess {
public:
	WindowsSubprocess();
	virtual ~WindowsSubprocess() override;
	virtual ProcessError start(const char *executeable, char *const *args) override;
	virtual ProcessError write_message(const void *buffer, size_t n) override;
	virtual ProcessError kill_process(int signal = 9) override;
	virtual char *read_buffer() override;
	virtual char *read_error_buffer() override;
};

#endif