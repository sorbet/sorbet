# typed: true

def foo(x,y,z); bar(x, y, z, ...); end # parser-error: unexpected token "..."
