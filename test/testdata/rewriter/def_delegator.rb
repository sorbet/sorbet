# typed: strict

class GoodUsages
  extend T::Sig
  extend Forwardable

  def_delegator :thing, :foo
  def_delegator :thing, :bar, :aliased_bar
  def_delegators :thing, :baz, :qux
  def_delegators :thing # Legal, but does nothing

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

    baz {}
    baz(1, 2)
    baz(1, 2) {}
    baz(1, 2, hey: true)
    baz(1, 2, hey: true) {}
    baz(hey: true)
    baz(hey: true) {}

    qux {}
    qux(1, 2)
    qux(1, 2) {}
    qux(1, 2, hey: true)
    qux(1, 2, hey: true) {}
    qux(hey: true)
    qux(hey: true) {}

    bar # error: Method `bar` does not exist
  end
end

class WorksWithoutExtendingTSig
  extend Forwardable

  def_delegator :thing, :foo
  def_delegator :thing, :bar, :aliased_bar
  def_delegators :thing, :baz, :qux
  def_delegators :thing
end

class ErrorsWhenMissingForwardable
  def_delegator :thing, :foo # error: Method `def_delegator` does not exist
  def_delegator :thing, :bar, :aliased_bar # error: Method `def_delegator` does not exist
  def_delegators :thing, :baz, :qux # error: Method `def_delegators` does not exist
  def_delegators :thing # error: Method `def_delegators` does not exist
end

class IgnoredUsages
  extend Forwardable

  local = 0
  not_def_delegator :thing, :foo # error: Method `not_def_delegator` does not exist
  def_delegator # error: Not enough arguments provided for method `Forwardable#def_delegator`
  def_delegator :thing
  #                   ^ error: Not enough arguments provided for method `Forwardable#def_delegator`
  def_delegator :thing, :foo, :bar, :baz
  #                                 ^^^^ error: Too many arguments provided for method `Forwardable#def_delegator`
  def_delegator :thing, :foo, :bar, kwarg: :thing
  #                                 ^^^^^^^^^^^^^ error: Too many arguments provided for method `Forwardable#def_delegator`
  def_delegator :thing, kwarg: :thing
  #                     ^^^^^^^^^^^^^ error: Expected `Symbol` but found `{kwarg: Symbol(:thing)}` for argument `method`
  def_delegator :foo, kwarg1: :thing, kwarg2: local
  #                   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Expected `Symbol` but found `{kwarg1: Symbol(:thing), kwarg2: Integer(0)}` for argument `method`
  def_delegator :foo, local => :thing
  #                   ^^^^^^^^^^^^^^^ error: Expected `Symbol` but found `{Integer(0) => Symbol(:thing)}` for argument `method`
  def_delegator 234, :foo
  #             ^^^ error: Expected `T.any(Symbol, String)` but found `Integer(234)` for argument `accessor`
  def_delegator :thing => :foo
  #                           ^ error: Not enough arguments provided for method `Forwardable#def_delegator
  #             ^^^^^^^^^^^^^^ error: Expected `T.any(Symbol, String)` but found `{thing: Symbol(:foo)}` for argument `accessor`


  def_delegators # error: Not enough arguments provided for method `Forwardable#def_delegators`
  def_delegators :thing, :foo, :bar, kwarg: :thing # error: Expected `Symbol` but found `{kwarg: Symbol(:thing)}` for argument `methods`
  def_delegators :foo, kwarg1: :thing, kwarg2: local # error: Expected `Symbol` but found `{kwarg1: Symbol(:thing), kwarg2: Integer(0)}` for argument `methods`
  def_delegators :foo, local => :thing # error: Expected `Symbol` but found `{Integer(0) => Symbol(:thing)}` for argument `methods`
  def_delegators 234, :foo # error: Expected `T.any(Symbol, String)` but found `Integer(234)` for argument `accessor`
  def_delegators :thing => :foo # error: Expected `T.any(Symbol, String)` but found `{thing: Symbol(:foo)}` for argument `accessor`
end

class EnumerableUsage
  extend T::Generic
  extend Forwardable

  include Enumerable

  Elem = type_member

  def_delegator :thing, :each
end
