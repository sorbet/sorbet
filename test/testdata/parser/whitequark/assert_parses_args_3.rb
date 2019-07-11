# typed: true

def f ((a, *r, p)); end # error-with-dupes: Unsupported rest args in destructure
