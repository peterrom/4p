#!/usr/bin/env python
# coding: utf-8

# Copyright 2016 Peter Román
# This file is part of 4p which is licensed under GNU GPL v3.
# See the file named LICENSE for details.

from subprocess import Popen, PIPE

from hypothesis import given, assume
from hypothesis.strategies import text


def parse(data):
    output, _ = Popen([u'./4p'], stdin=PIPE, stdout=PIPE) \
        .communicate(data.encode(u'utf-8'))

    return output.decode(u'utf-8')


@given(text())
def test_no_cmd(data):
    assume(u'/*$' not in data)

    assert parse(data) == data


@given(text())
def test_single_cmd(data):
    assume(u' $*/' not in data)
    assume(u'EOF' not in data)

    assert \
        parse(u'/*$ cat <<EOF\n{}EOF $*/'.format(data)) == u'{}'.format(data)


if __name__ == '__main__':
    import sys

    def is_test(v):
        try:
            return v.func_name.startswith(u'test_')
        except AttributeError:
            return False

    tests = (v for v in globals().copy().itervalues() if is_test(v))

    for test in tests:
        print test.func_name, u'...',

        try:
            test()
        except AssertionError:
            print u'Fail'
        except:
            print u'Error'
        else:
            print u'Pass'
        finally:
            sys.stdout.flush()
