# typed: true

def foo; bar(*); end # parser-error: no anonymous rest parameter
