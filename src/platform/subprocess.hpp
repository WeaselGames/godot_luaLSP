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
		ALREADY_RUNNING,
		NOTHING_TO_READ,
		FAILED_TO_KILL,
		FAILED_TO_WRITE,
		FAILED_TO_READ,
		FALIED_TO_START,
		FAILED_TO_CREATE_PIPE,
	};

	enum Status {
		RUNNING,
		STOPPED,
		ERROR_STATE
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

	inline bool is_running() const {
		return running;
	};

	inline static std::string error_string(Error error) {
		switch (error) {
			case OK:
				return "No error";
			case NOT_RUNNING:
				return "Process is not running";
			case ALREADY_RUNNING:
				return "Process is already running";
			case NOTHING_TO_READ:
				return "Nothing to read";
			case FAILED_TO_KILL:
				return "Failed to kill process";
			case FAILED_TO_WRITE:
				return "Failed to write to process";
			case FAILED_TO_READ:
				return "Failed to read from process";
			case FALIED_TO_START:
				return "Failed to start process";
			case FAILED_TO_CREATE_PIPE:
				return "Failed to create pipe";
			default:
				return "Unknown error";
		}
	}

	static Subprocess *new_platform_subprocess();
};

#endif