# typed: false

class A; end
#     ^ type-def: A

class TestClass
  puts A
  #    ^ type: A
end
