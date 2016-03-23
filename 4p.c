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

struct buf_stream {
        char *buf;
        size_t sz;
};

struct buf_stream buf_stream_next(const struct buf_stream s)
{
        return (struct buf_stream) {s.buf + 1, s.sz - 1};
};

bool buf_stream_eol(const struct buf_stream s)
{
        return s.sz == 0;
}

const char *find_in_buf(const char *const buf, const size_t buf_sz,
                        const char *const substr)
{
        size_t looking_for = 0;

        for (size_t i = 0; i < buf_sz; ++i) {
                char this = substr[looking_for];

                if (this == '\0')
                        return buf + i;

                if (buf[i] == this)
                        ++looking_for;
                else
                        looking_for = 0;
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
