# typed: true

def foo(@@abc); end # parser-error: formal argument cannot be a class variable
