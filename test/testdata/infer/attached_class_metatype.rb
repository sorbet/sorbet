# typed: true

class Parent
  extend T::Sig

  sig {returns(T::Array[T.attached_class])}
  def self.make_multi
    T.attached_class.to_s # error: Call to method `to_s` on `T.untyped` mistakes a type for a value

    xs = T::Array[T.attached_class].new

    xs << new
    xs << new

    # Should actually be `T::Array[T.attached_class (of Parent)]`
    # See https://github.com/sorbet/sorbet/issues/4352
    T.reveal_type(xs) # error: Revealed type: `T::Array[T.untyped]`
  end
end

class Child < Parent; end

T.reveal_type(Parent.make_multi) # error: Revealed type: `T::Array[Parent]`
T.reveal_type(Child.make_multi) # error: Revealed type: `T::Array[Child]`
