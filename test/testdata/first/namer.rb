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
  include Mixin, Mixin # warning
  include 3 # warning
  include Mixin do
  end # warning
  whatever.include OtherMixin
end
