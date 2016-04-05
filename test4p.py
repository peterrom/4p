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
