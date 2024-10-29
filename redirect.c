//Alex Nguyen

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_error(const char *message) {
    fprintf(stderr, "%s: %lu\n", message, GetLastError());
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: redirect <inp> <cmd> <out>\n");
        return 1;
    }

    char *inp = argv[1];
    char *cmd = argv[2]; // Command without quotes
    char *out = argv[3]; // Output file
    char cmdLine[1024] = {0}; // Command line to execute

    // Prepare command line
    snprintf(cmdLine, sizeof(cmdLine), "cmd /c \"%s\"", cmd);  // Use cmd /c to run the command

    HANDLE hInput = INVALID_HANDLE_VALUE;
    HANDLE hOutput = INVALID_HANDLE_VALUE;

    // Open input file if `inp` is not "-"
    if (strcmp(inp, "-") != 0) {
        hInput = CreateFile(inp, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hInput == INVALID_HANDLE_VALUE) {
            print_error("Error opening input file");
            return 1;
        }
    }

    // Open output file if `out` is not "-"
    if (strcmp(out, "-") != 0) {
        hOutput = CreateFile(out, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hOutput == INVALID_HANDLE_VALUE) {
            print_error("Error opening output file");
            if (hInput != INVALID_HANDLE_VALUE) CloseHandle(hInput);
            return 1;
        }
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;

    // Redirect input and output
    si.hStdInput = (hInput != INVALID_HANDLE_VALUE) ? hInput : GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = (hOutput != INVALID_HANDLE_VALUE) ? hOutput : GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = si.hStdOutput;

    printf("Executing command: %s\n", cmdLine);  // Debugging output

    // Create the process
    ZeroMemory(&pi, sizeof(pi));
    if (!CreateProcess(NULL, cmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        print_error("Error creating process");
        if (hInput != INVALID_HANDLE_VALUE) CloseHandle(hInput);
        if (hOutput != INVALID_HANDLE_VALUE) CloseHandle(hOutput);
        return 1;
    }

    // Wait for the process to complete
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Get exit code
    DWORD exitCode;
    if (!GetExitCodeProcess(pi.hProcess, &exitCode)) {
        print_error("Error getting exit code");
    } else {
        printf("Process exit code: %lu\n", exitCode);
        if (exitCode != 0) {
            fprintf(stderr, "Command failed with exit code: %lu\n", exitCode);
        }
    }

    // Close handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    if (hInput != INVALID_HANDLE_VALUE) CloseHandle(hInput);
    if (hOutput != INVALID_HANDLE_VALUE) CloseHandle(hOutput);

    return 0;
}
