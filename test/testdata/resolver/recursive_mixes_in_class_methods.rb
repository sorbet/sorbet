# typed: true

# Test recursion support with 3 levels

module Foo
  extend T::Helpers
  module ClassMethods
    def foo; end
  end
  mixes_in_class_methods(ClassMethods)
  def a; end
end

module Bar
  extend T::Helpers
  include Foo
  module ClassMethods
    def bar; end
  end
  mixes_in_class_methods(ClassMethods)
end

module Baz
  extend T::Helpers
  include Bar
  module ClassMethods
    def baz; end
  end
  mixes_in_class_methods(ClassMethods)

  def a_class_method; end
end

class Qux
  include Baz
end

Qux.foo
Qux.bar
Qux.baz
Qux.new.a
Qux.a_class_method # error: Method `a_class_method` does not exist on `T.class_of(Qux)`

# Test recursion and regular linearization together

module UnusedMixin
  def unused; end
end

module Quux
  extend T::Helpers
  include UnusedMixin
  module ClassMethods
    def quux; end
  end
  mixes_in_class_methods(ClassMethods)
end

module Corge
  def corge; end
end

class Grault
  include Quux
  extend Corge
end

Grault.quux
Grault.unused # error: Method `unused` does not exist on `T.class_of(Grault)`
Grault.corge
