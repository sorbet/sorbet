# typed: true

def foo(args, &)
  bar(*args, &)
end

def bar(args, &)
  baz(&)
end

foo (1) { 2 }
