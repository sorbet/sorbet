# typed: true

def foo(a = 1 + "2") # error: Expected `Integer` but found `String("2")` for argument `arg0`
end
