# typed: true

def foo
  if defined?(A::B)
    puts A::B
  end
end

foo
module A
  class B
  end
end
foo
