# typed: true

class C
  extend T::Sig
  sig {final.void} # error: The syntax for declaring a method final is `sig(:final) {...}`, not `sig {final. ...}`
  def foo; end
end
