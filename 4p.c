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
        const char *t = substr;

        for (const char *c = buf; c < buf + buf_sz; ++c) {
                if (*t == '\0') {
                        return c;
                } else if (*c == *t) {
                        ++t;
                } else if (*c == *substr) {
                        t = substr + 1;
                } else {
                        t = substr;
                }
        }

        return NULL;
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
