# typed: true

class Foo::MyModel
  include T::Props
  prop :array_of_explicit, T::Array[String]
end
