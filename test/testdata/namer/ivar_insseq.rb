# typed: true

class A
  extend T::Sig
  sig { returns(Integer) }
  def foo
    @bar ||= begin; x = 1; T.let(x, Integer); end
  # ^^^^ error: The instance variable `@bar` must be declared inside `initialize` or declared nilable
  # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: This code is unreachable
  end

  def other
    T.reveal_type(@bar) # error: `Integer`
  end
end
