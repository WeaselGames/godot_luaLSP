#if defined(LCE_PLATFORM_POSIX)

#include "subprocess.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

Subprocess *Subprocess::new_platform_subprocess() {
	return new PosixSubprocess();
}

PosixSubprocess::PosixSubprocess() {
}

PosixSubprocess::~PosixSubprocess() {
	if (running) {
		Error err = this->kill_process();
		if (err != OK) {
			// TODO: Handle error
		}
	}
}

#include <iostream>

Subprocess::Error PosixSubprocess::start(std::string executeable, std::vector<std::string> args) {
	if (running) {
		return ALREADY_RUNNING;
	}

	if (stdio_pipe[0] != 0) {
		close(stdio_pipe[0]);
	}
	if (stdio_pipe[1] != 0) {
		close(stdio_pipe[1]);
	}
	if (stderr_pipe[0] != 0) {
		close(stderr_pipe[0]);
	}
	if (stderr_pipe[1] != 0) {
		close(stderr_pipe[1]);
	}

	if (pipe(stdio_pipe) == -1) {
		return FAILED_TO_CREATE_PIPE;
	}

	if (pipe(stderr_pipe) == -1) {
		return FAILED_TO_CREATE_PIPE;
	}

	int flags = fcntl(stdio_pipe[0], F_GETFD);
	fcntl(stdio_pipe[0], F_SETFL, flags | O_NONBLOCK);

	flags = fcntl(stderr_pipe[0], F_GETFD);
	fcntl(stderr_pipe[0], F_SETFL, flags | O_NONBLOCK);

	proccess_pid = fork();
	if (proccess_pid == 0) { // Child process
		dup2(stdio_pipe[0], STDIN_FILENO);
		dup2(stdio_pipe[1], STDOUT_FILENO);
		dup2(stderr_pipe[1], STDERR_FILENO);
		close(stderr_pipe[0]);
		char **args_c = (char **)malloc(sizeof(char *) * (args.size() + 1));
		for (int i = 0; i < args.size(); i++) {
			args_c[i] = (char *)args[i].c_str();
		}
		args_c[args.size()] = nullptr;
		execvp(executeable.c_str(), args_c);
		delete[] args_c;
		close(stdio_pipe[0]);
		close(stdio_pipe[1]);
		close(stderr_pipe[1]);
	} else {
		close(stderr_pipe[1]);
		running = true;
	}

	return OK;
}

Subprocess::Error PosixSubprocess::write_message(std::string message) {
	if (!running) {
		return NOT_RUNNING;
	}

	int ret = write(stdio_pipe[1], message.c_str(), message.length());
	if (ret == -1) {
		return FAILED_TO_WRITE;
	}

	return OK;
}

Subprocess::Error PosixSubprocess::kill_process() {
	if (!running) {
		return NOT_RUNNING;
	}

	int status;
	kill(proccess_pid, SIGKILL);
	waitpid(proccess_pid, &status, 0);
	if (WIFEXITED(status)) {
		running = false;
		return OK;
	} else if (WIFSIGNALED(status)) {
		running = false;
		return OK;
	}

	return FAILED_TO_KILL;
}

Subprocess::Status PosixSubprocess::get_status() {
	if (!running) {
		return STOPPED;
	}

	int ret = waitpid(proccess_pid, nullptr, WNOHANG);
	if (ret > 0) {
		return RUNNING;
	} else if (ret == -1) {
		return ERROR_STATE;
	}

	return STOPPED;
}

std::pair<std::string, PosixSubprocess::Error> PosixSubprocess::read_output(int nbytes) {
	if (!running) {
		return std::make_pair("", NOT_RUNNING);
	}

	char *content = (char *)malloc(nbytes);
	int ret = read(stdio_pipe[0], content, nbytes);
	if (ret == -1) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			return std::make_pair("", FAILED_TO_READ);
		}

		return std::make_pair("", NOTHING_TO_READ);
	}

	std::string content_string = std::string(content, ret);
	free(content);

	return std::make_pair(content_string, OK);
}

std::pair<char, PosixSubprocess::Error> PosixSubprocess::read_output_char() {
	if (!running) {
		return std::make_pair('\0', NOT_RUNNING);
	}

	char _char = '\0';
	int nbytes = read(stdio_pipe[0], &_char, 1);
	if (nbytes == -1) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			return std::make_pair('\0', FAILED_TO_READ);
		}

		return std::make_pair('\0', NOTHING_TO_READ);
	}

	return std::make_pair(_char, OK);
}

std::pair<std::string, PosixSubprocess::Error> PosixSubprocess::read_error(int nbytes) {
	if (!running) {
		return std::make_pair("", NOT_RUNNING);
	}

	char *content = (char *)malloc(nbytes);
	int ret = read(stderr_pipe[0], content, nbytes);
	if (ret == -1) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			return std::make_pair("", FAILED_TO_READ);
		}

		return std::make_pair("", NOTHING_TO_READ);
	}

	std::string content_string = std::string(content, ret);
	free(content);

	return std::make_pair(content_string, OK);
}

std::pair<char, PosixSubprocess::Error> PosixSubprocess::read_error_char() {
	if (!running) {
		return std::make_pair('\0', NOT_RUNNING);
	}

	char _char = '\0';
	int nbytes = read(stderr_pipe[0], &_char, 1);
	if (nbytes == -1) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			return std::make_pair('\0', FAILED_TO_READ);
		}

		return std::make_pair('\0', NOTHING_TO_READ);
	}

	return std::make_pair(_char, OK);
}

#endif