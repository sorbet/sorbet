# @typed
module Mixin
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
  extend T::Helpers
  mixes_in_class_methods # error: Wrong number of arguments to mixes_in_class_methods
end

class Bad2
  extend T::Helpers
  module ClassMethods; end
  mixes_in_class_methods(ClassMethods) # error: can only be declared inside a module
end

module Bad3
  extend T::Helpers

  class ClassMethods; end
  mixes_in_class_methods(ClassMethods) # error: is a class, not a module
end

module Bad4
  extend T::Helpers

  module ClassMethods; end
  mixes_in_class_methods(ClassMethods)
  mixes_in_class_methods(ClassMethods) # error: can only be declared once
end

module Bad5
  extend T::Helpers

  mixes_in_class_methods(0) # error: must be statically resolvable to a module
end

module Bad6
  extend T::Helpers
  mixes_in_class_methods(Bad6) # error: Must not pass your self
end
