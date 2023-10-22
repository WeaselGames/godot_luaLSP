#ifndef WINDOWS_SUBPROCESS_H
#define WINDOWS_SUBPROCESS_H

#include "../subprocess.h"

class WindowsSubprocess : public Subprocess {
public:
	WindowsSubprocess();
	virtual ~WindowsSubprocess() override;
	virtual Error start(std::string executeable, std::vector<std::string> args) override;
	virtual Error write_message(std::string message) override;
	virtual Error kill_process() override;
	virtual Status get_status() override;

	virtual std::pair<std::string, Error> read_output(int nbytes) override;
	virtual std::pair<char, Error> read_output_char() override;
	virtual std::pair<std::string, Error> read_error(int nbytes) override;
	virtual std::pair<char, Error> read_error_char() override;
};

#endif