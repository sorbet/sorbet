# typed: true
# Spacer for assert fast path
# Spacer to allow for exclude from file update assertion

class A
  extend T::Sig
  sig {returns(Integer)}
  def to_method
    0
  end
end
