# typed: true

def foo(foo: bar { || foo }); end

m { |foo = proc { || 1 + foo }| }

def foo(foo: bar { || baz }); end

m { |foo = proc { || 1 + bar }| }
