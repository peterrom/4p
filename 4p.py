#!/usr/bin/env python3

# Copyright 2015 Peter Román
# This file is part of 4p which is licensed under GNU GPL v3.
# See the file named LICENSE for details.

# This file contains extremely crude prototype code.

import re
import subprocess


def parse(s):
    m = re.search('(\$+\()', s)

    if not m:
        return [s]

    a = s[:m.start()]
    b, c = s[m.end():].split(')' + m.group().strip('('), 1)

    return [a, b] + parse(c)


def main(ppfile):
    assert ppfile.endswith('.4p')

    s = open(ppfile).read()
    f = open(ppfile[:-3], mode='w')

    p = parse(s) + ['']

    for a, b in zip(p[0::2], p[1::2]):
        f.write(a + subprocess.check_output(['/bin/bash',
                                             '-c',
                                             b])
                .decode('utf-8')
                .strip('\n'))

if __name__ == '__main__':
    import sys
    main(sys.argv[1])
