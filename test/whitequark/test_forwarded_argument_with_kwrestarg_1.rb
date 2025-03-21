# typed: true

def foo; bar(argument, **); end # parser-error: no anonymous keyword rest parameter
