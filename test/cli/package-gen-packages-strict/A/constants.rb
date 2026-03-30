# typed: strict

module A
  CONSTANT_FROM_A = "Hello from Package A"
  puts B::CONSTANT_FROM_B
  puts B::ANOTHER_CONSTANT_FROM_B
end
