#!/usr/bin/env python
# coding: utf-8

# Copyright 2016 Peter Román
# This file is part of 4p which is licensed under GNU GPL v3.
# See the file named LICENSE for details.

from subprocess import Popen, PIPE

from hypothesis import given, assume
from hypothesis.strategies import text


def parse(data):
    output, _ = Popen(['./4p'], stdin=PIPE, stdout=PIPE) \
        .communicate(data.encode('utf-8'))

    return output.decode('utf-8')


@given(text())
def test_no_cmd(data):
    assume(u'/*$' not in data)

    assert parse(data) == data


if __name__ == '__main__':
    import sys

    def is_test(v):
        try:
            return v.func_name.startswith('test_')
        except AttributeError:
            return False

    tests = (v for v in globals().copy().itervalues() if is_test(v))

    for test in tests:
        print test.func_name, '...',

        try:
            test()
        except AssertionError:
            print 'Fail'
        except:
            print 'Error'
        else:
            print 'Pass'
        finally:
            sys.stdout.flush()
