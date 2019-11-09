# typed: true

def foo
  if defined?(A::B)
    puts A::B
  else
    puts "not defined"
  end
end

foo
module A
  class B
  end
end
foo

puts defined?(A+2)
# puts defined?(1)
