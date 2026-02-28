# typed: strict

class AbstractClass
  extend T::Helpers

  T.unsafe(self).some_dsl
end

AbstractClass.new
