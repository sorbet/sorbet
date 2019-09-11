# typed: true

module Arrow
  extend T::Sig
  extend T::Generic
  extend T::Helpers

  interface!

  Arg = type_member(:in)
  Result = type_member(:out)

  sig {returns(Arrow[T.untyped, T.untyped])}
  def self.make
    T.unsafe(nil)
  end

  sig {abstract.params(x: Arg).returns(Result)}
  def call(x); end
end

class Unit; end

module OnlyIn
  extend T::Sig
  extend T::Generic
  extend T::Helpers

  interface!

  In = type_member(:in)

  # should pass: `In` occurs in the covariant position of Arrow, which is in
  # contravariant position.
  sig {abstract.params(arr: Arrow[Unit, In]).void}
  def arrow_ok(arr); end

  # should fail: `In` occurs in the contravariant position of Arrow, which is in
  # contravariant position.
  sig {abstract.params(arr: Arrow[In, Unit]).void}
                     # ^^^ error: `type_member` `In` was defined as `:in` but is used in an `:out` context
  def arrow_fail(arr); end

  # should pass: `In` occurs in the contravariant position of Arrow, which is in
  # covariant position.
  sig {abstract.returns(Arrow[In,Unit])}
  def ret_arrow_ok; end

  # should fail: `In` occurs in the covariant position of Arrow, which is in
  # covariant position.
  sig {abstract.returns(Arrow[Unit,In])}
  def ret_arrow_fail; end
# ^^^^^^^^^^^^^^^^^^ error: `type_member` `In` was defined as `:in` but is used in an `:out` context
end

module OnlyOut
  extend T::Sig
  extend T::Generic
  extend T::Helpers

  interface!

  Out = type_member(:out)

  # should fail: `Out` occurs in the covariant position of Arrow, which is in
  # contravariant position.
  sig {abstract.params(arr: Arrow[Unit, Out]).void}
                     # ^^^ error: `type_member` `Out` was defined as `:out` but is used in an `:in` context
  def arrow_fail(arr); end

  # should pass: `Out` occurs in the contravariant position of Arrow, which is
  # in contravariant position.
  sig {abstract.params(arr: Arrow[Out, Unit]).void}
  def arrow_ok(arr); end

  # should fail: `Out` occurs in the contravariant position of Arrow, which is
  # in covariant position.
  sig {abstract.returns(Arrow[Out,Unit])}
  def ret_arrow_fail; end
# ^^^^^^^^^^^^^^^^^^ error: `type_member` `Out` was defined as `:out` but is used in an `:in` context

  # should pass: `Out` occurs in the covariant position of Arrow, which is in
  # covariant position.
  sig {abstract.returns(Arrow[Unit,Out])}
  def ret_arrow_ok; end
end
