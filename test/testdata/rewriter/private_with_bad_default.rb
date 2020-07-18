# typed: strict

class Main
  extend T::Sig

  sig {params(arg0: T.nilable(String)).returns(NilClass)}
  private def private_with_bad_default(arg0: 0) # error: Argument does not have asserted type `T.nilable(String)`
  end

  sig {params(arg0: T.nilable(String)).returns(NilClass)}
  def public_with_bad_default(arg0: 0) # error: Argument does not have asserted type `T.nilable(String)`
  end
end
