# typed: true
# compiled: true
module A
  module B
  end
  class C
  end
end
module A::D
end
class A::E
end

puts A::B.class
puts A::C.class
puts A::D.class
puts A::E.class
