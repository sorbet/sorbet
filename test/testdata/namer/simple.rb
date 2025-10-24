# typed: true
class NormalClass
  def normal_method
  end
  def self.normal_static_method
  end
  class InnerClass
  end
  module InnerModule
  end
end

module ANamespace
  class ObviousChild
  end
end
class ANamespace::ClassInNamespace
end

class Parent
end
module Mixin
end
module OtherMixin
end
class Child < Parent
  include Mixin
  include 3
  #       ^ error: `include` must only contain constant literals
  #       ^ error: Expected `T::Module[T.anything]` but found `Integer(3)` for argument `arg0`
  include Mixin do; end
  #             ^^^^^^^ error: Method `Module#include` does not take a block
# ^^^^^^^^^^^^^^^^^^^^^ error: `include` can not be passed a block
  whatever.include OtherMixin # error: Method `whatever` does not exist on `T.class_of(Child)`
end
