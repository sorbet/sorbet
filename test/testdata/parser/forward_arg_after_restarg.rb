# typed: true

def foo *rest, ...; end # parser-error: ... after rest argument
