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
class Child < Parent
end
