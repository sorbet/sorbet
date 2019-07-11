# typed: true

def f ((*r, p)); end # error-with-dupes: Unsupported rest args in destructure
