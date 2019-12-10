# typed: true

class Child
  extend T::Sig

  sig {returns(T.experimental_attached_class)}
  def self.foo
    # Goal of test: no < ... > in the user-visible type.
    T.reveal_type(new) # error: Revealed type: `T.attached_class (of Child)`
  end
end
