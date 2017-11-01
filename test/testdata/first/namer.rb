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
  include Mixin, Mixin # :error:
  include 3 # :error:
  include Mixin do
  end # :error:
  whatever.include OtherMixin
end
