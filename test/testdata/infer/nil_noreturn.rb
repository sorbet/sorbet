# typed: true

class Nil
  extend T::Sig
  extend T::Generic
  # It's not a dead code error to have T.noreturn for a type_member
  Elem = type_member {{fixed: T.noreturn}}

  # But it is if you ever try to do so much as load an arg with that type. (The
  # load arg instruction would be dead, indicating the method can't be called.)
  sig {params(x: Elem).void}
  def foo(x)
    puts x # error: This code is unreachable
  end
end
