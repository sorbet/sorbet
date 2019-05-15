# typed: true

class Parent
  def self.foo; end
end

Class.new
Class.new(Parent)

Class.new {|cls| cls.superclass}
Class.new(Parent) {|cls| cls.superclass}

Class.new(Parent).foo
c = Class.new(Parent) do |cls|
  # These doesn't typecheck because we don't solve typeconstaints before we
  # typecheck the block (by our choice, to leave the user with flexibility to
  # affect type parameters inside the block), so we can't use
  # `T.type_parameter(:T)` as the block's param's type or the block's bind.
  # cls.foo
  # foo
end
c.foo
