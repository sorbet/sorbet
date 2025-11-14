# typed: false

def f1(**nil, **nil); end # error: unexpected token ","
def f2(*args, **kwargs, **nil); end # error: unexpected token ","
def f3(*args, a:, **nil); end # error: unexpected token "nil"
def f4(a:, **nil); end # error: unexpected token "nil"
def f5(**nil: 10); end # error: unexpected token tLABEL
