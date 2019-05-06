# typed: true

# Originally: https://github.com/stripe/sorbet/issues/399

Parent = Struct.new(:foo)
class Child < Parent
  def hello
  end
end

T.reveal_type(Child.new) # Revealed type: `Child`
Child.new.hello

class B < Struct
  Elem = type_member(fixed: T.untyped)
  def initialize
  end
end

T.reveal_type(B.new) # Revealed type: `B`
