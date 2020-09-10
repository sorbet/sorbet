# typed: true

def f1(**nil); end
def f2(*args, **nil); end
def f3(a, b, c, **nil); end
def f4(a, b, *c, **nil); end
def f5(**nil, &blk); end
def f6(a, b, c, **nil, &blk); end
def f7(a, b, *c, **nil, &blk); end
