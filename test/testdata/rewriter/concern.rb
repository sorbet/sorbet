# typed: true
module ActiveSupport::Concern
  # Dummy definition
end

# Test with class_methods is used
module Foo
  extend ActiveSupport::Concern
  class_methods do
    extend T::Sig

    sig { void }
    def a_class_method
    end
  end

  def instance_method
  end
end

class A
  include Foo
end

A.new.instance_method
A.a_class_method
A.new.a_class_method # error: Method `a_class_method` does not exist on `A`

# Test with ClassMethods is used
module Bar
  extend ActiveSupport::Concern
  module ClassMethods
    extend T::Sig

    sig { void }
    def a_class_method
    end

    def another_class_method
    end
  end

  def instance_method
  end
end

class B
  include Bar
end

B.new.instance_method
B.a_class_method
B.another_class_method
B.new.a_class_method # error: Method `a_class_method` does not exist on `B`

# Test with both `ClassMethods` and `class_methods` being used
module Baz
  extend ActiveSupport::Concern
  module ClassMethods
    def a_class_method
    end
  end

  class_methods do
    extend T::Sig

    sig { void }
    def a_class_method_2
    end
  end

  def instance_method
  end
end

class C
  include Baz
end

C.new.instance_method
C.a_class_method
C.a_class_method_2

# Same as above but used in different order
module Qux
  extend ActiveSupport::Concern
  class_methods do
    def a_class_method
    end
  end

  module ClassMethods
    def a_class_method_2
    end
  end

  def instance_method
  end
end

class D
  include Qux
end

D.new.instance_method
D.a_class_method
D.a_class_method_2

# Test no ClassMethods module
module Quux
  extend ActiveSupport::Concern
  def instance_method
  end
end

class E
  include Quux
end

E.new.instance_method

# Test without extend ActiveSupport::Concern
module Corge
  class_methods do # error: Method `class_methods` does not exist on `T.class_of(Corge)`
    def a_class_method
    end
  end

  def instance_method
  end
end

class F
  include Corge
end

F.new.instance_method
F.a_class_method # error: Method `a_class_method` does not exist on `T.class_of(F)`

# Test with double class_methods definition
module Grault
  extend ActiveSupport::Concern
  class_methods do
    def a_class_method
    end
  end

  class_methods do
    def another_class_method
    end
  end
end

class G
  include Grault
end

G.a_class_method
G.another_class_method
