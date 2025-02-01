// IPC types:
// 1. Shared memory
// 2. File
// 3. Sockets
// 4. Channels (named, anonymous)
// 5. Mutexes
// 6. Signals
// 7. Message queues

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef NDEBUG
    #define DEBUG_PRINT(fmt, ...) fprintf(stdout, "DEBUG: %s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...)
#endif

enum ReqArgType {
    REQ_ARG_NONE,
    REQ_ARG_IPC_TYPE,
    REQ_ARG_FILE
};

enum IPCType {
    IPC_NONE,
    IPC_SHARED_MEMORY,
    IPC_FILE,
    IPC_SOCKETS,
    IPC_CHANNELS,
    IPC_MUTEXES,
    IPC_SIGNALS,
    IPC_MESSAGE_QUEUES
};

void IPC_file(char* file)
{

}

int main(int argc, char* argv[])
{
    enum IPCType IPC_type = IPC_NONE;
    char* file = NULL;

    if (argc < 3) {
        printf("Arguments missing\n");
        return -1;
    }

    char flags[][20] = {
        "--ipc_type",
        "--file"
    };
    bool reqs[] = {
        true,
        true
    };

    size_t flags_count = sizeof(flags) / sizeof(flags[0]);

    char ipc_names[][20] = {"shared_memory", "file", "sockets", "channels", "mutexes", "signals", "message_queues"};
    size_t ipc_names_count = sizeof(ipc_names) / sizeof(ipc_names[0]);

    enum ReqArgType req_arg = REQ_ARG_NONE;
    for (size_t i = 1; i < argc; i++) {

        if (req_arg == REQ_ARG_NONE) {
            bool have_flag = false;
            for (size_t j = 0; j < flags_count; j++) {
                if (strcmp(argv[i], flags[j]) == 0) {
                    req_arg = reqs[j] * ((enum ReqArgType) (j + 1));
                    have_flag = true;
                    break;
                }
            }
            if (!have_flag) {
                printf("Invalid flag: %s\n", argv[i]);
                return -1;
            }
        } else {
            if (req_arg == REQ_ARG_IPC_TYPE) {
                for (size_t j = 0; j < ipc_names_count; j++) {
                    if (strcmp(argv[i], ipc_names[j]) == 0) {
                        IPC_type = (enum IPCType) (j + 1);
                        req_arg = REQ_ARG_NONE;
                        break;
                    }
                }
                if (IPC_type == IPC_NONE) {
                    printf("Invalid IPC type: %s\n", argv[i]);
                    return -1;
                }
            }
            
            if (req_arg == REQ_ARG_FILE) {
                file = argv[i];
                req_arg = REQ_ARG_NONE;
            }
        }
    }

    DEBUG_PRINT("IPC type: %s", ipc_names[IPC_type - 1]);

    switch (IPC_type) {

        case IPC_SHARED_MEMORY:
            break;

        case IPC_FILE:
            DEBUG_PRINT("File: %s", file);
            IPC_file(file);
            break;

        case IPC_SOCKETS:
            break;

        case IPC_CHANNELS:
            break;

        case IPC_MUTEXES:
            break;

        case IPC_SIGNALS:
            break;

        case IPC_MESSAGE_QUEUES:
            break;
            
        default:
            break;
    }

    return 0;
}
