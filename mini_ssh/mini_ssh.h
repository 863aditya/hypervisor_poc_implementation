#ifndef MINI_SSH_H
#define MINI_SSH_H

#include "../db/db.h"

// Declarations for the mini_ssh client and server implementations.
// These functions are implemented in `client.cpp` and `server.cpp` respectively.
// They return 0 on success and non-zero on error (conventional CLI style).
int client_run(int argc, char** argv, int SERVER_PORT = 2222);
int server_run(int argc, char** argv, int SERVER_PORT = 2222);

// If `shell.c` provides C functions that C++ code needs to link to,
// keep the extern "C" wrapper to ensure correct linkage.
#ifdef __cplusplus
extern "C" {
#endif
    void run_shell(); // optional: implemented in shell.c if present
#ifdef __cplusplus
}
#endif

#endif // MINI_SSH_H