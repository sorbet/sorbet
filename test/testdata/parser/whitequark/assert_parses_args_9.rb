# typed: true

def f ((*, p)); end # error-with-dupes: Unsupported rest args in destructure
