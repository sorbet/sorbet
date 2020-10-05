# typed: true

module Qux
  extend T::Helpers
  module ClassMethods
    def qux; end
  end
  mixes_in_class_methods(ClassMethods)
end

module Foo
  extend T::Helpers
  include Qux
  module ClassMethods
    def foo; end
  end
  mixes_in_class_methods(ClassMethods)
  def a; end
end

module Bar
  extend T::Helpers
  # extend ActiveSupport::Concern # TODO: Do we need this as a requirement for this functionality?
  include Foo
  module ClassMethods
    def bar; end
  end
  mixes_in_class_methods(ClassMethods)
end

class Baz
  include Bar
end

Baz.bar
Baz.foo
Baz.qux
Baz.new.a






# module Foo
#   extend ActiveSupport::Concern
#   class_methods do
#     def foo; end
#   end
# end

# module Bar
#   extend ActiveSupport::Concern
#   include Foo

#   included do
#     foo
#   end
# end

# class Baz
#   include Bar
# end

# Baz.foo
