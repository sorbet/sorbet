# typed: strict

class GoodUsages
  extend T::Sig
  extend Forwardable

  def_delegator :thing, :foo
  def_delegator :thing, :bar, :aliased_bar

  sig {void}
  def usages
    foo {}
    foo(1, 2)
    foo(1, 2) {}
    foo(1, 2, hey: true)
    foo(1, 2, hey: true) {}
    foo(hey: true)
    foo(hey: true) {}

    aliased_bar {}
    aliased_bar(1, 2)
    aliased_bar(1, 2) {}
    aliased_bar(1, 2, hey: true)
    aliased_bar(1, 2, hey: true) {}
    aliased_bar(hey: true)
    aliased_bar(hey: true) {}

    bar # error: Method `bar` does not exist
  end
end

class WorksWithoutExtendingTSig
  def_delegator :thing, :foo
  def_delegator :thing, :bar, :aliased_bar
end

class IgnoredUsages
  extend Forwardable

  local = 0
  not_def_delegator :thing, :foo # error: Method `not_def_delegator` does not exist
  def_delegator # error: Not enough arguments provided for method Forwardable#def_delegator. Expected: 2..3, got: 0
  def_delegator :thing # error: Not enough arguments provided for method Forwardable#def_delegator. Expected: 2..3, got: 0
  def_delegator :thing, :foo, :bar, :baz # error: Too many arguments provided for method Forwardable#def_delegator. Expected: 2..3, got: 4
  def_delegator :thing, :foo, :bar, kwarg: :thing # error: Too many arguments provided for method Forwardable#def_delegator. Expected: 2..3, got: 4
  def_delegator :thing, kwarg: :thing # error: Expected Symbol but found {kwarg: Symbol(:"thing")} for argument method
  def_delegator :foo, kwarg1: :thing, kwarg2: local # error: Expected Symbol but found {kwarg1: Symbol(:"thing"), kwarg2: Integer(0)} for argument method
  def_delegator :foo, local => :thing # error: Expected Symbol but found {} for argument method
  def_delegator 234, :foo # error: Expected T.any(Symbol, String) but found Integer(234) for argument accessor
  def_delegator :thing => :thing # error: Method `def_delegator` does not exist
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Not enough arguments provided for method Forwardable#def_delegator. Expected: 2..3, got: 1
end
