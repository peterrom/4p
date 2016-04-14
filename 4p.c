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

const char *find_in_buf(const char *const buf, const size_t buf_sz,
                        const char *const substr)
{
        const char *const end = buf + buf_sz;
        const char *c = buf;
        const char *t = substr;

        for (; c != end; ++c) {
                if (*t == '\0') {
                        return c - strlen(substr);
                } else if (*c == *t) {
                        ++t;
                } else if (*c == *substr) {
                        t = substr + 1;
                } else {
                        t = substr;
                }
        }

        return c;
}

const char *handle_text(const char *const beg, const char *const end)
{
        const char *text_end = find_in_buf(beg, end - beg, "/*$");
        retrying_write(STDOUT_FILENO, beg, text_end - beg);
        return text_end;
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

#define FSM
#define STATE(name) fsm_ ## name:
#define NEXTSTATE(name) goto fsm_ ## name

void parse(void)
{
        char buf[16];
        char *beg = buf;
        const char *end = buf + sizeof(buf);

        FSM {
                STATE(parsing_text) {
                        char *pos;
                        for (pos = beg; pos < end && *pos != '/'; ++pos);
                        retrying_write(STDOUT_FILENO, beg, end - pos);

                        if (pos == end)
                                NEXTSTATE(empty_buf_in_text);
                        else
                                NEXTSTATE(cmd_start);
                }

                STATE(empty_buf_in_text) {
                        // fill buf
                        NEXTSTATE(parsing_text);
                }

                STATE(cmd_start) {
                        NEXTSTATE(parsing_cmd);
                }
        }
}

int main(void)
{
        parse();
        return 0;
}
