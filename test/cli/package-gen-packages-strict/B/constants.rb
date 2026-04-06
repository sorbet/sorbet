# typed: strict

module B
  CONSTANT_FROM_B = "Hello from Package B"
  ANOTHER_CONSTANT_FROM_B = "Hello from Package B"
  puts A::CONSTANT_FROM_A
  puts C::CONSTANT_FROM_C
end
