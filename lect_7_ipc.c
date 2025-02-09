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
#include <sys/types.h>
#include <sys/mman.h>

#ifndef NDEBUG
    #define DEBUG_PRINT(fmt, ...) fprintf(stdout, "DEBUG: %s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...)
#endif

int IPC_shared_memory(bool is_host, char* shared_mem_file, size_t size)
{
    if (shared_mem_file == NULL) {
        printf("Error invalid file argument\n");
        return -1;
    }

    if (shared_mem_file[0] != '/') {
        printf("Error file not have slash symbol\n");
        return -1;
    }

    for (size_t i = 1; shared_mem_file[i] != '\0'; i++) {
        if (shared_mem_file[i] == '/') {
            printf("Error file have slash after first symbol\n");
            return -1;
        }
    }

    int shared_mem_fd = shm_open(shared_mem_file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (shared_mem_fd < 0) {
        perror("Cannot open shared memory file");
        return -1;
    }

    if (is_host) {

    }

    return 0;
}

int IPC_file(char* file)
{
    if (file == NULL) {
        printf("Error invalid file argument\n");
        return -1;
    }

    int fd = open(file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    if (fd < 0) {
        perror("Error opening file");
        return -1;
    }

    struct termios old, no_blockable;
    tcgetattr(STDIN_FILENO, &old);
    no_blockable = old;
    no_blockable.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &no_blockable);

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
    lock.l_start = 0;

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

            fcntl(fd, F_SETLKW, &lock);

            ssize_t writed_to_file = write(fd, write_buf, write_buf_size);

            lock.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lock);
            lock.l_type = F_WRLCK;

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

enum FLagType {
    FLAG_IPC_TYPE,
    FLAG_FILE,
    FLAG_SIZE,
    FLAG_IS_HOST
};

#define FLAGS_COUNT 4

enum ReqArgType {
    REQ_ARG_NONE,
    REQ_ARG_IPC_TYPE,
    REQ_ARG_FILE,
    REQ_ARG_SIZE
};

#define ARGS_COUNT 3

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

#define IPC_COUNT 7

int main(int argc, char* argv[])
{
    enum IPCType arg_IPC_type = IPC_NONE;
    char* arg_file = NULL;
    size_t arg_size = 0;
    bool arg_is_host = false;

    if (argc < 3) {
        printf("Arguments missing\n");
        return -1;
    }

    char flags[FLAGS_COUNT][20] = {
        "--ipc_type",
        "--file",
        "--size",
        "--is_host"
    };
    bool reqs[FLAGS_COUNT] = {
        true,
        true,
        true,
        false
    };
    bool indicated_flags[FLAGS_COUNT] = {false};

    char ipc_names[IPC_COUNT][20] = {
        "shared_memory",
        "file",
        "sockets",
        "channels",
        "mutexes",
        "signals",
        "message_queues"
    };

    enum ReqArgType req_arg = REQ_ARG_NONE;
    for (size_t i = 1; i < argc; i++) {

        if (req_arg == REQ_ARG_NONE) {
            bool have_flag = false;
            for (size_t j = 0; j < FLAGS_COUNT; j++) {
                if (strcmp(argv[i], flags[j]) == 0) {
                    req_arg = reqs[j] * ((enum ReqArgType) (j + 1));
                    indicated_flags[j] = true;
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
                for (size_t j = 0; j < IPC_COUNT; j++) {
                    if (strcmp(argv[i], ipc_names[j]) == 0) {
                        arg_IPC_type = (enum IPCType) (j + 1);
                        req_arg = REQ_ARG_NONE;
                        break;
                    }
                }
                if (arg_IPC_type == IPC_NONE) {
                    printf("Invalid IPC type: %s\n", argv[i]);
                    return -1;
                }
            }
            
            if (req_arg == REQ_ARG_FILE) {
                arg_file = argv[i];
                req_arg = REQ_ARG_NONE;
            }
            size_t x;
            if (req_arg == REQ_ARG_SIZE) {
                arg_size = (size_t) strtoul(argv[i], NULL, 10);
                req_arg = REQ_ARG_NONE;
            }
        }
    }

    DEBUG_PRINT("IPC type: %s", ipc_names[arg_IPC_type - 1]);
    int ret = 0;

    switch (arg_IPC_type) {

        case IPC_SHARED_MEMORY:
            DEBUG_PRINT(
                "Is host: %s, File: %s, size: %zu",
                indicated_flags[FLAG_IS_HOST]? "true" : "false",
                arg_file,
                arg_size);
            ret = IPC_shared_memory(indicated_flags[FLAG_IS_HOST], arg_file, arg_size);
            break;

        case IPC_FILE:
            DEBUG_PRINT("File: %s", arg_file);
            ret = IPC_file(arg_file);
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
