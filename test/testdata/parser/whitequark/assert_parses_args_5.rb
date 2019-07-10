# typed: true

def f ((a, *, p)); end # error-with-dupes: Unsupported rest args in destructure
