# typed: false

# This test is designed to simulate what would happen with sealed in the
# presence of weird srb init RBI generation.

module M
  extend T::Helpers

  sealed!
end

class A
  include M

  define_method(:from_a) do
  end
end

class B
  include M

  define_method(:from_b) do
  end
end

class C
  include M

  define_method(:from_c) do
  end
end
