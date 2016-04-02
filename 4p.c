/* Copyright 2016 Peter Román
   This file is part of 4p which is licensed under GNU GPL v3.
   See the file named LICENSE for details. */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

void exit_on_bad_errno(void)
{
        switch (errno) {
        case EAGAIN: // == EWOULDBLOCK
                break;
        default:
                printf("%s\n", strerror(errno));
                exit(1);
        }
}

size_t retrying_read(char *const buf, const size_t buf_sz)
{
        ssize_t sz;

        while ((sz = read(STDIN_FILENO, buf, buf_sz)) == -1)
                exit_on_bad_errno();

        return (size_t) sz;
}

void retrying_write(const char *const buf, const size_t buf_sz) {
        size_t written = 0;

        while (written < buf_sz) {
                ssize_t sz = write(
                        STDOUT_FILENO, buf + written, buf_sz - written);

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
        retrying_write(beg, text_end - beg);
        return text_end;
}

void parse(void)
{
        char buf[1024];
        size_t sz;

        while ((sz = retrying_read(buf, sizeof(buf)))) {
                retrying_write(buf, sz);
        }
}

int main(void)
{
        parse();
        return 0;
}
