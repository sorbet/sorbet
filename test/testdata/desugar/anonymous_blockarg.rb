# typed: true

def foo(args, &)
  bar(*args, &)
end

def bar(args, &)
  baz(&) # error: Method `baz` does not exist on `Object`
end

foo (1) { 2 }
