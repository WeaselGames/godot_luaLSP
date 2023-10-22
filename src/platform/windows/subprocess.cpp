#if defined(LCE_PLATFORM_WINDOWS)

#include "subprocess.h"

#include <iostream>

Subprocess *Subprocess::new_platform_subprocess() {
	return new WindowsSubprocess();
}

WindowsSubprocess::WindowsSubprocess() {
}

WindowsSubprocess::~WindowsSubprocess() {
	if (running) {
		Error err = this->kill_process();
		if (err != OK) {
			// TODO: Handle error
		}
	}
}

WindowsSubprocess::Error WindowsSubprocess::start(std::string executable, std::vector<std::string> args) {
	if (running) {
		return ALREADY_RUNNING;
	}

	if (stdio_pipe[0] != 0) {
		CloseHandle(stdio_pipe[0]);
	}

	if (stdio_pipe[1] != 0) {
		CloseHandle(stdio_pipe[1]);
	}

	if (stderr_pipe[0] != 0) {
		CloseHandle(stderr_pipe[0]);
	}

	if (stderr_pipe[1] != 0) {
		CloseHandle(stderr_pipe[1]);
	}

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	if (!CreatePipe(&stdio_pipe[0], &stdio_pipe[1], &sa, 0)) {
		return FAILED_TO_CREATE_PIPE;
	}

	if (!CreatePipe(&stderr_pipe[0], &stderr_pipe[1], &sa, 0)) {
		return FAILED_TO_CREATE_PIPE;
	}

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.hStdInput = stdio_pipe[0];
	si.hStdOutput = stdio_pipe[1];
	si.hStdError = stderr_pipe[1];
	si.dwFlags |= STARTF_USESTDHANDLES;

	std::string args_str = executable;
	for (int i = 0; i < args.size(); i++) {
		args_str += " " + args[i];
	}

	if (!CreateProcess(nullptr, (LPSTR)args_str.c_str(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &process_info)) {
		return FALIED_TO_START;
	}

	running = true;
	CloseHandle(stderr_pipe[1]);
	stderr_pipe[1] = 0;

	return OK;
}

WindowsSubprocess::Error WindowsSubprocess::write_message(std::string message) {
	if (!running) {
		return NOT_RUNNING;
	}

	if (!WriteFile(stdio_pipe[1], message.c_str(), message.size(), nullptr, nullptr)) {
		return FAILED_TO_WRITE;
	}

	return OK;
}

WindowsSubprocess::Error WindowsSubprocess::kill_process() {
	if (!running) {
		return NOT_RUNNING;
	}

	if (!TerminateProcess(process_info.hProcess, 0)) {
		return FAILED_TO_KILL;
	}

	running = false;

	CloseHandle(stdio_pipe[1]);
	stdio_pipe[1] = 0;
	CloseHandle(stdio_pipe[0]);
	stdio_pipe[0] = 0;
	CloseHandle(stderr_pipe[0]);
	stderr_pipe[0] = 0;

	return OK;
}

WindowsSubprocess::Status WindowsSubprocess::get_status() {
	if (!running) {
		return STOPPED;
	}

	DWORD exit_code;
	if (!GetExitCodeProcess(process_info.hProcess, &exit_code)) {
		return ERROR_STATE;
	}

	if (exit_code == STILL_ACTIVE) {
		return RUNNING;
	}

	return STOPPED;
}

std::pair<std::string, WindowsSubprocess::Error> WindowsSubprocess::read_output(int nbytes) {
	if (!running) {
		return std::make_pair("", NOT_RUNNING);
	}

	char *buffer = new char[nbytes];
	DWORD bytes_read;
	if (!ReadFile(stdio_pipe[0], buffer, nbytes, &bytes_read, nullptr)) {
		return std::make_pair("", FAILED_TO_READ);
	}

	std::string ret(buffer, bytes_read);
	delete[] buffer;

	return std::make_pair(ret, OK);
}

std::pair<char, WindowsSubprocess::Error> WindowsSubprocess::read_output_char() {
	if (!running) {
		return std::make_pair('\0', NOT_RUNNING);
	}

	char buffer;
	DWORD bytes_read;
	if (!ReadFile(stdio_pipe[0], &buffer, 1, &bytes_read, nullptr)) {
		return std::make_pair('\0', FAILED_TO_READ);
	}

	return std::make_pair(buffer, OK);
}

std::pair<std::string, WindowsSubprocess::Error> WindowsSubprocess::read_error(int nbytes) {
	if (!running) {
		return std::make_pair("", NOT_RUNNING);
	}

	char *buffer = new char[nbytes];
	DWORD bytes_read;
	if (!ReadFile(stderr_pipe[0], buffer, nbytes, &bytes_read, nullptr)) {
		return std::make_pair("", FAILED_TO_READ);
	}

	std::string ret(buffer, bytes_read);
	delete[] buffer;

	return std::make_pair(ret, OK);
}

std::pair<char, WindowsSubprocess::Error> WindowsSubprocess::read_error_char() {
	if (!running) {
		return std::make_pair('\0', NOT_RUNNING);
	}

	char buffer;
	DWORD bytes_read;
	if (!ReadFile(stderr_pipe[0], &buffer, 1, &bytes_read, nullptr)) {
		return std::make_pair('\0', FAILED_TO_READ);
	}

	return std::make_pair(buffer, OK);
}

#endif