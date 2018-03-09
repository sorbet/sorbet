# @typed
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
  include Mixin, Mixin # error: `include` should only be passed a single constant. You passed `2` parameters.
  include 3 # error: `include` must be passed a constant literal.
  include Mixin do # error: `include` can not be passed a block.
  end
  whatever.include OtherMixin
end
