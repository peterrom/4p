/* Copyright 2016 Peter Román
   This file is part of 4p which is licensed under GNU GPL v3.
   See the file named LICENSE for details. */

#include <sys/types.h>
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

int bash(void)
{
        int pipefd[2];
        pid_t pid;

        if (pipe(pipefd) == -1) {
                exit_with_errno();
        }

        if ((pid = fork()) == -1) {
                exit_with_errno();
        }

        if (pid != 0) {
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

struct buffer {
        char data[16];
        char *pos;
        char *end;
};

void buffer_init(struct buffer *buf)
{
        buf->pos = buf->data;
        buf->end = buf->data + sizeof(buf->data);
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

        return buf->end > buf->pos;
}

int main(void)
{
        struct buffer buf;
        buffer_init(&buf);

        const char *delimiters[] = {"/*$ ", " $*/"};
        int dindex = 0;
        const char *looking_for = delimiters[dindex];

        while (buffer_replenish(&buf)) {
                for (; buf.pos < buf.end; ++buf.pos) {
                        if (*buf.pos == *looking_for)
                                ++looking_for;
                        else
                                looking_for = delimiters[dindex];

                        if (*looking_for == '\0') {
                                dindex = (dindex + 1) % 2;
                                looking_for = delimiters[dindex];
                        }
                }
        }

        return 0;
}
