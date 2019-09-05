# typed: true

class A; end
#     ^ type-def: A

class TestClass
  a = A.new
# ^ type: A
  puts a
end
