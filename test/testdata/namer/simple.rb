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
  include 3 # error: `include` must only contain simple expressions
  include Mixin do # error: `include` can not be passed a block
  end
  whatever.include OtherMixin # error: Method `whatever` does not exist on `T.class_of(Child)`
end
