# typed: true

def f ((a, *r)); end # error-with-dupes: Unsupported rest args in destructure
