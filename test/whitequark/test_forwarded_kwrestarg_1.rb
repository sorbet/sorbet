# typed: true

def foo; bar(**); end # parser-error: no anonymous keyword rest parameter
