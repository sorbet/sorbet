# typed: true

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

Baz.bar  # should work
Baz.foo  # should work
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
