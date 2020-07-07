# typed: true

class Abstract
  extend T::Sig
  extend T::Helpers
  abstract!

  sig { abstract.params(fail: Integer).returns(Integer) }
  def foo(fail:); end
end

class Pass < Abstract
  extend T::Sig

  sig { override.params(fail: Integer).returns(Integer) }
  def foo(fail: nil)
              # ^^^ error: Argument does not have asserted type `Integer`
    5
  end

  sig { params(fail: Integer).returns(Integer) }
  def bar(fail: nil)
              # ^^^ error: Argument does not have asserted type `Integer`
    5
  end
end
