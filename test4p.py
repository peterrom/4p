from subprocess import Popen, PIPE


def parse(data):
    output, _ = Popen(['./4p'], stdin=PIPE, stdout=PIPE).communicate(data)
    return output


def test_no_cmd():
    data = 'abcdefg'
    assert parse(data) == data
