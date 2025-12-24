# typed: true

module Root
  module ClassMethods
    def test
    end
  end

  # This looks like extending into a symbol defined by `Root::Subpackge`, as
  # it's a dependency of this package but also defines that module as part of
  # its namespace.
  extend ClassMethods
end
