# typed: true
extend T::Sig

class Parent; end
class Child < Parent; end

class FinalClass
  extend T::Helpers
  final!
end

sig { params(x: T.any(Parent, Integer)).void }
def example1(x)
  if x.instance_of?(Parent)
    T.reveal_type(x) # error: `Parent`
  else
    T.reveal_type(x) # error: `T.any(Parent, Integer)`
  end
end

sig { params(x: T.any(FinalClass, Integer)).void }
def example2(x)
  if x.instance_of?(FinalClass)
    T.reveal_type(x) # error: `FinalClass`
  else
    T.reveal_type(x) # error: `Integer`
  end
end
