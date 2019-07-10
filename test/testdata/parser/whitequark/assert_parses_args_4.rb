# typed: true

def f ((a, *)); end # error-with-dupes: Unsupported rest args in destructure
