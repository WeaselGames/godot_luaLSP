#ifndef POSIX_SUBPROCESS_H
#define POSIX_SUBPROCESS_H

#include "../subprocess.h"
#include <unistd.h>

class PosixSubprocess : public Subprocess {
public:
	PosixSubprocess();
	virtual ~PosixSubprocess() override;
	virtual Error start(std::string executeable, std::vector<std::string> args) override;
	virtual Error write_message(std::string message) override;
	virtual Error kill_process() override;
	virtual Status get_status() override;

	virtual std::pair<std::string, Error> read_output(int nbytes) override;
	virtual std::pair<char, Error> read_output_char() override;
	virtual std::pair<std::string, Error> read_error(int nbytes) override;
	virtual std::pair<char, Error> read_error_char() override;

private:
	int stdio_pipe[2];
	int stderr_pipe[2];
	pid_t proccess_pid = 0;
};

#endif