# typed: true

# Test no recursion for T::Helpers#mixes_in_class_methods

module NoRecursion
  module Foo
    extend T::Helpers
    module ClassMethods
      def foo; end
    end
    mixes_in_class_methods(ClassMethods)
    def instance_method; end
    def self.class_method; end
  end

  module Bar
    extend T::Helpers
    include Foo
    module ClassMethods
      def bar1; end
    end
    module ClassMethods
      def bar2; end
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
  end

  class Qux
    include Baz
  end

  Qux.foo            # error: Method `foo` does not exist on `T.class_of(NoRecursion::Qux)`
  Qux.bar1           # error: Method `bar1` does not exist on `T.class_of(NoRecursion::Qux)`
  Qux.bar2           # error: Method `bar2` does not exist on `T.class_of(NoRecursion::Qux)`
  Qux.baz
  Qux.new.instance_method
  Qux.class_method   # error: Method `class_method` does not exist on `T.class_of(NoRecursion::Qux)`
end

# Test recursion support with 3 levels

module ActiveSupport::Concern
end

module RecursionSupport
  module Foo
    extend ActiveSupport::Concern
    module ClassMethods
      def foo; end
    end
    def instance_method; end
    def self.class_method; end
  end

  module Bar
    extend ActiveSupport::Concern
    include Foo
    module ClassMethods
      def bar1; end
    end
    module ClassMethods
      def bar2; end
    end
  end

  module Baz
    extend ActiveSupport::Concern
    include Bar
    module ClassMethods
      def baz; end
    end
  end

  class Qux
    include Baz
  end

  Qux.foo
  Qux.bar1
  Qux.bar2
  Qux.baz
  Qux.new.instance_method
  Qux.class_method # error: Method `class_method` does not exist on `T.class_of(RecursionSupport::Qux)`
end

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
