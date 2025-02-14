# typed: true

def foo; bar(argument, *); end # parser-error: no anonymous rest parameter
