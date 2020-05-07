# typed: strict

module BagOfProps
  include T::Props

  prop :foo, String
end

class MyClass
  include BagOfProps
end

my_class = MyClass.new
p my_class.foo = 'hello'
p my_class.foo
