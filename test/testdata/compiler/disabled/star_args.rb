# typed: true

def foo(*args)
  puts args.inspect
end
foo(1)
foo(1,2,3)
