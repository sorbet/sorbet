# typed: true

def f1(**nil, **nil); end # parser-error: unexpected token ","
def f2(*args, **kwargs, **nil); end # parser-error: unexpected token ","
def f3(*args, a:, **nil); end # parser-error: unexpected token "nil"
def f4(a:, **nil); end # parser-error: unexpected token "nil"
def f5(**nil: 10); end # parser-error: unexpected token tLABEL
