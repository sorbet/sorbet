# typed: false

foo(x: 0)
foo({x: 0})
foo(**{x: 0})
foo(**hash)

def foo(x:, x:)
          # ^^ error: duplicate argument name x
end

def foo(x:, x: 10)
          # ^^^^^ error: duplicate argument name x
end
