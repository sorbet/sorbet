# typed: true

class Parent
  extend T::Sig

  sig {returns(Integer)}
  def parent_bar; 1; end
end

class Child < Parent
  alias_method(:child_bar, :parent_bar)
end

T.reveal_type(Child.new.child_bar) # Revealed type: `Integer`

# --- again, but in a different order ---

class ChildFirst < ParentSecond
  alias_method(:foo, :bar)
end

class ParentSecond
  extend T::Sig

  sig {returns(NilClass)}
  def bar
  end
end

T.reveal_type(ChildFirst.new.foo) # Revealed type: `NilClass`
