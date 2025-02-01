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
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

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

int IPC_shared_memory()
{

    return 0;
}

int IPC_file(char* file)
{
    if (file == NULL) {
        printf("Invalid file argument\n");
        return -1;
    }

    int fd = open(file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        perror("Error opening file");
        return -1;
    }

    struct termios old, no_blockable;
    tcgetattr(STDIN_FILENO, &old);
    no_blockable = old;
    no_blockable.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &no_blockable);

    fd_set fds;
    struct timeval tv = {0, 0};

    char read_buf[1024];
    size_t read_buf_capacity = sizeof(read_buf) / sizeof(read_buf[0]);
    char write_buf[1024];
    size_t write_buf_capacity = sizeof(read_buf) / sizeof(write_buf[0]);

    pid_t pid = getpid();

    int ret = 0;
    do {
        size_t write_buf_size = 0;

        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        int key_pressed = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);

        if (key_pressed) {

            char ch;
            if (read(STDIN_FILENO, &ch, 1) < 0) {
                perror("Error read command");
                ret = -1;
                break;
            }

            if (ch == '\n')
                break;

            printf("write line: %c", ch);
            fflush(stdout);

            size_t cur_char = snprintf(write_buf, write_buf_capacity, "process %d: ", pid);
            write_buf[cur_char++] = ch;

            ssize_t input = 0;
            while ((input = read(STDIN_FILENO, write_buf + cur_char, 1)) >= 0) {

                write(STDOUT_FILENO, write_buf + cur_char, 1);

                if (cur_char == write_buf_capacity - 2 || write_buf[cur_char] == '\n')
                    break;
                cur_char++;
            }

            if (input < 0) {
                perror("Error input text");
                ret = -1;
                break;
            }

            cur_char++;
            write_buf[cur_char] = '\n';
            write_buf_size = cur_char;
        }

        ssize_t read_bytes = read(fd, read_buf, read_buf_capacity);
        if (read_bytes < 0) {
            perror("Error reading file");
            ret = -1;
            break;
        }
        if (read_bytes > 0) {
            write(STDOUT_FILENO, read_buf, read_bytes);
        }

        if (write_buf_size) {
            ssize_t writed_to_file = write(fd, write_buf, write_buf_size);
            if (writed_to_file < 0) {
                perror("Error cannot write to file");
                return -1;
            }
        }

        usleep(1000);

    } while (true);

    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    close(fd);
    return ret;
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
    int ret = 0;

    switch (IPC_type) {

        case IPC_SHARED_MEMORY:
            ret = IPC_shared_memory();
            break;

        case IPC_FILE:
            DEBUG_PRINT("File: %s", file);
            ret = IPC_file(file);
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

    return ret;
}
