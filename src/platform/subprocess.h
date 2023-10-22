#ifndef SUBPROCESS_H
#define SUBPROCESS_H

#include <string>
#include <utility>
#include <vector>

class Subprocess {
protected:
	bool running = false;

public:
	enum Error {
		OK,
		NOT_RUNNING,
		ALREAD_RUNNING,
		NOTHING_TO_READ,
		FAILED_TO_KILL,
		FAILED_TO_WRITE,
		FAILED_TO_READ,
		FAILED_TO_CREATE_PIPE,
	};

	enum Status {
		RUNNING,
		STOPPED,
		ERROR
	};

	virtual ~Subprocess() = default;
	virtual Error start(std::string executeable, std::vector<std::string> args) = 0;
	virtual Error write_message(std::string message) = 0;
	virtual Error kill_process() = 0;
	virtual Status get_status() = 0;

	virtual std::pair<std::string, Error> read_output(int nbytes) = 0;
	virtual std::pair<char, Error> read_output_char() = 0;
	virtual std::pair<std::string, Error> read_error(int nbytes) = 0;
	virtual std::pair<char, Error> read_error_char() = 0;

	bool is_running() const;

	static std::string error_to_string(Error error);
	static Subprocess *new_platform_subprocess();
};

#endif