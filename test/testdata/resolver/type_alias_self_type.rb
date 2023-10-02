# typed: strict

class Example
  extend T::Sig

  # This magically becomes "the selftype of wherever this type alias is used"
  # instead of "the self type of `self` right here" (i.e., the singleton class)
  # I'm not sure if this behavior is intentional or accidental, but it is what
  # it is so now we have a test for it. At least we don't crash.
  X = T.type_alias { T.self_type }

  sig { returns(X) }
  def example
    self
  end

  sig { returns(X) }
  def self.example_singleton
    self
  end
end

x = Example.new.example
T.reveal_type(x) # error: `Example`

y = Example.example_singleton
T.reveal_type(y) # error: `T.class_of(Example)`
