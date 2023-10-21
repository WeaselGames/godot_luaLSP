#include "subprocess.h"

#include <fcntl.h>
#include <malloc.h>
#include <signal.h>
#include <stdio.h>

PosixSubprocess::PosixSubprocess() {
	buffer[0] = '\0';
	error_buffer[0] = '\0';
}

PosixSubprocess::~PosixSubprocess() {
	if (running) {
		ProcessError err = this->kill_process();
		if (err != OK) {
			// TODO: Handle error
		}
	}
}

#include <iostream>

Subprocess::ProcessError PosixSubprocess::start(const char *executeable, char *const *args) {
	if (running) {
		return ALREAD_RUNNING;
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

	flags = fcntl(stdio_pipe[1], F_GETFD);
	fcntl(stdio_pipe[1], F_SETFL, flags | O_NONBLOCK);

	proccess_pid = fork();
	if (proccess_pid == 0) { // Child process
		dup2(stdio_pipe[0], STDIN_FILENO);
		dup2(stdio_pipe[1], STDOUT_FILENO);
		dup2(stderr_pipe[1], STDERR_FILENO);
		close(stderr_pipe[0]);
		write(STDOUT_FILENO, "Hello", 5);
		execvp(executeable, args);
		close(stdio_pipe[0]);
		close(stdio_pipe[1]);
		close(stderr_pipe[1]);
	} else {
		close(stderr_pipe[1]);
		running = true;
	}

	return OK;
}

Subprocess::ProcessError PosixSubprocess::write_message(const void *buffer, size_t n) {
	if (!running) {
		return NOT_RUNNING;
	}

	int ret = write(stdio_pipe[1], buffer, n);
	if (ret == -1) {
		return FAILED_TO_WRITE;
	}

	return OK;
}

Subprocess::ProcessError PosixSubprocess::poll() {
	if (!running) {
		return NOT_RUNNING;
	}
	char _char = '\0';
	int amount_read = 0;

	int error_read_amount = (SUBPROCESS_BUFFER_SIZE - 1) - current_error_buffer_size;
	if (error_read_amount <= 0) {
		return BUFFER_FULL;
	}

	int nbytes = read(stderr_pipe[0], &_char, 1);
	while (nbytes > 0) {
		amount_read++;
		error_buffer[current_error_buffer_size] = _char;
		current_error_buffer_size++;

		if (current_buffer_size >= SUBPROCESS_BUFFER_SIZE - 1) {
			return BUFFER_FULL;
		}

		nbytes = read(stderr_pipe[0], &_char, 1);
	}

	if (nbytes == -1) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			return FAILED_TO_READ;
		}
	}

	int read_amount = (SUBPROCESS_BUFFER_SIZE - 1) - current_buffer_size;
	if (read_amount <= 0) {
		return BUFFER_FULL;
	}

	nbytes = read(stdio_pipe[0], &_char, 1);

	while (nbytes > 0) {
		amount_read++;
		buffer[current_buffer_size] = _char;
		current_buffer_size++;

		if (current_buffer_size >= SUBPROCESS_BUFFER_SIZE - 1) {
			return BUFFER_FULL;
		}

		nbytes = read(stdio_pipe[0], &_char, 1);
	}

	if (nbytes == -1) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			return FAILED_TO_READ;
		}
	}

	if (nbytes == 0) {
		return NOTHING_TO_READ;
	}

	return OK;
}

Subprocess::ProcessError PosixSubprocess::kill_process(int signal) {
	if (!running) {
		return NOT_RUNNING;
	}

	int ret = kill(proccess_pid, signal);
	if (ret == 0) {
		running = false;
		return OK;
	}

	close(stdio_pipe[0]);
	close(stdio_pipe[1]);
	close(stderr_pipe[0]);

	return FAILED_TO_KILL;
}

char *PosixSubprocess::read_buffer() {
	if (!running) {
		return nullptr;
	}

	if (current_buffer_size == 0) {
		std::cout << "No buffer to read" << std::endl;
		return nullptr;
	}

	char *buffer_content = (char *)malloc(current_buffer_size);
	for (int i = 0; i < current_buffer_size; i++) {
		buffer_content[i] = buffer[i];
	}

	current_buffer_size = 0;
	buffer[0] = '\0';

	return buffer_content;
}

char *PosixSubprocess::read_error_buffer() {
	if (!running) {
		return nullptr;
	}

	char *buffer_content = (char *)malloc(current_error_buffer_size);
	for (int i = 0; i < current_error_buffer_size; i++) {
		buffer_content[i] = error_buffer[i];
	}

	current_error_buffer_size = 0;
	error_buffer[0] = '\0';

	return buffer_content;
}