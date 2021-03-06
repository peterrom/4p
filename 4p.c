/* Copyright 2016 Peter Román
   This file is part of 4p which is licensed under GNU GPL v3.
   See the file named LICENSE for details. */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

void exit_with_errno(void)
{
        printf("%s\n", strerror(errno));
        exit(1);
}

void exit_on_bad_errno(void)
{
        switch (errno) {
        case EAGAIN: // == EWOULDBLOCK
                break;
        default:
                exit_with_errno();
        }
}

size_t retrying_read(char *const buf, const size_t buf_sz)
{
        ssize_t sz;

        while ((sz = read(STDIN_FILENO, buf, buf_sz)) == -1)
                exit_on_bad_errno();

        return (size_t) sz;
}

void retrying_write(const int fd, const char *const buf, const size_t buf_sz) {
        size_t written = 0;

        while (written < buf_sz) {
                const ssize_t sz = write(fd, buf + written, buf_sz - written);

                if (sz == -1) {
                        exit_on_bad_errno();
                        continue;
                }

                written += sz;
        }
}

int bash_stream(void)
{
        int pipefd[2];
        pid_t pid;

        if (pipe(pipefd) == -1) {
                exit_with_errno();
        }

        if ((pid = fork()) == -1) {
                exit_with_errno();
        }

        if (pid == 0) {
                close(pipefd[1]);
                dup2(pipefd[0], STDIN_FILENO);

                execve("/bin/bash",
                       (char *const[]) {"/bin/bash", NULL},
                       NULL);

                exit_with_errno();
        }

        close(pipefd[0]);
        return pipefd[1];
}

int stdout_stream(void)
{
        return STDOUT_FILENO;
}

void close_stream(int fd)
{
        if (fd != STDOUT_FILENO) {
                close(fd);

                int status;
                do {
                        wait(&status);
                } while (!WIFEXITED(status));
        }
}

struct buffer {
        char data[16];
        char *pos;
        char *end;
};

void buffer_init(struct buffer *buf)
{
        buf->end = buf->data;
        buf->pos = buf->data;
}

size_t buffer_available(struct buffer *buf)
{
        return buf->end - buf->pos;
}

bool buffer_replenish(struct buffer *buf)
{
        const size_t save_sz = buffer_available(buf);
        memmove(buf->data, buf->pos, save_sz);

        const size_t read_sz = retrying_read(buf->data + save_sz,
                                             sizeof(buf->data) - save_sz);

        buf->pos = buf->data;
        buf->end = buf->data + save_sz + read_sz;

        return read_sz;
}

bool matches(const char *a, const char *b, size_t n)
{
        for (; n && *a == *b; --n, ++a, ++b);
        return !n;
}

ssize_t parse(struct buffer *buf, const char *const looking_for)
{
        for (; buf->pos < buf->end; ++buf->pos) {
                if (*buf->pos == *looking_for) {
                        const size_t sz = strlen(looking_for);

                        if (buffer_available(buf) < sz) {
                                return -1;
                        } else if (matches(buf->pos, looking_for, sz)) {
                                return sz;
                        }
                }
        }

        return 0;
}

int main(void)
{
        struct buffer buf;
        buffer_init(&buf);

        int (*streams[])(void) = {stdout_stream, bash_stream};
        const char *delimiters[] = {"/*$ ", " $*/"};

        int dindex = 0;

        int stream = streams[dindex]();
        const char *looking_for = delimiters[dindex];

        for (;;) {
                const ssize_t res = parse(&buf, looking_for);

                retrying_write(stream, buf.data, buf.pos - buf.data);

                switch (res) {
                case -1:
                        if (buffer_replenish(&buf) == 0) {
                                retrying_write(stream, buf.data, buffer_available(&buf));
                                exit(0);
                        }

                        break;
                case 0:
                        if (buffer_replenish(&buf) == 0)
                                exit(0);

                        break;
                default:
                        buf.pos += res;

                        close_stream(stream);

                        dindex ^= 1;
                        stream = streams[dindex]();
                        looking_for = delimiters[dindex];

                        (void) buffer_replenish(&buf);
                        break;
                }
        }

        return 0;
}
