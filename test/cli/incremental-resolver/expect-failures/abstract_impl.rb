# typed:true
class A
  extend T::Sig
  sig { overridable.void }
  def foo(*foo)end
end
class B < A
 def foo; end
end
