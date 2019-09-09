# typed: false

class A; end
#     ^ type-def: A

class TestClass
  puts A
  #    ^ type: A

  # Doesn't work since inference doesn't run on this file
  a = A.new
  puts a
  #    ^ type: (nothing)
end
