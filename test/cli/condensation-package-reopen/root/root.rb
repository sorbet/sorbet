# typed: true

class Root < Prelude::A
  module ClassMethods
    def test
    end
  end

  # This looks like extending into a symbol defined by `Root::Subpackge`, as
  # it's a dependency of this package but also defines that module as part of
  # its namespace.
  extend ClassMethods

  # Same
  include ClassMethods

  X = type_member

  Y = type_template
end
