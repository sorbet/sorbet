# typed: true

# Pretend this is an RBI file that declares only a sig

class A
  extend T::Sig

  sig {returns(Integer)}
  def foo; 0; end
end
