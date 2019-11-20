# typed: true

def foo
  if defined?(A::B)
    puts A::B
  end
end

foo
module A
  class B
    # puts defined?(B)
  end
end
foo

puts defined?(A)
# puts defined?(1)
puts defined?(A+1)
puts defined?(B+1)
