# typed: true
module Mixin
  extend T::Sig
  extend T::Helpers

  module ClassMethods
    def mixin_class_method
    end
  end

  mixes_in_class_methods(ClassMethods)

  def mixin_method
  end
end

class Test
  include Mixin
end

Test.mixin_class_method
Test.new.mixin_method


module Bad1
  extend T::Sig
  extend T::Helpers
  mixes_in_class_methods # error: Not enough arguments provided for method `T::Helpers#mixes_in_class_methods`. Expected: `1+`, got: `0`
end

class Bad2
  extend T::Sig
  extend T::Helpers
  module ClassMethods; end
  mixes_in_class_methods(ClassMethods) # error: can only be declared inside a module
end

module Bad3
  extend T::Sig
  extend T::Helpers

  class ClassMethods; end
  mixes_in_class_methods(ClassMethods) # error: is a class, not a module
end

module Bad4
  extend T::Sig
  extend T::Helpers

  mixes_in_class_methods(0) # error: must be statically resolvable to a module
  #                      ^ error: Expected `Module` but found `Integer(0)`
end

module Bad5
  extend T::Sig
  extend T::Helpers

  RUBY_CONSTANT = 0
  mixes_in_class_methods(RUBY_CONSTANT) # error: must be statically resolvable to a module
  #                      ^^^^^^^^^^^^^ error: Expected `Module` but found `Integer`
end

module Bad6
  extend T::Sig
  extend T::Helpers
  mixes_in_class_methods(Bad6) # error: Must not pass your self
end
