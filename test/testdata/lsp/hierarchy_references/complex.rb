# typed: true
class Module; include T::Sig; end

class Parent
  extend T::Helpers
  abstract!
  sig { abstract.returns(Integer) }
  def foo; end
  #   ^ find-hierarchy-refs: Parent#foo
  #   ^ hierarchy-ref: Child#foo
  #   ^ hierarchy-ref: ChildViaProp#foo
end

module IFoo
  extend T::Helpers
  abstract!
  sig { abstract.returns(Integer) }
  def foo; end
  #   ^ hierarchy-ref: Child#foo
  #   ^ no Parent#foo hierarchy ref here, because this is outside the hierarchy
  #   ^ no ChildViaProp hierarchy ref here, because it doesn't include IFoo
end

class EmptyInBetween < Parent
  abstract!
end

class Child < EmptyInBetween
  include IFoo
  sig { override.returns(Integer) }
  def foo; 0; end
  #   ^ hierarchy-ref: Parent#foo
  #   ^ find-hierarchy-refs: Child#foo
  #   ^ hierarchy-ref: ChildViaProp#foo
end

class ChildViaProp < Parent
  include T::Props

  prop :foo, Integer, override: :reader
  # ^ hierarchy-ref: Parent#foo
  # ^ hierarchy-ref: Child#foo
  #     ^ find-hierarchy-refs: ChildViaProp#foo

  sig { params(foo: Integer).void }
  def initialize(foo:)
    @foo = foo
    #^ hierarchy-ref: Parent#foo
    #^ hierarchy-ref: Child#foo
    #^ hierarchy-ref: ChildViaProp#foo
  end

  def example
    @foo
    #^ hierarchy-ref: Parent#foo
    #^ hierarchy-ref: Child#foo
    #^ hierarchy-ref: ChildViaProp#foo
  end
end

class GrandChild < Child
  sig { override.returns(Integer) }
  def foo; 0; end
  #   ^ hierarchy-ref: Parent#foo
  #   ^ hierarchy-ref: Child#foo
  #   ^ hierarchy-ref: ChildViaProp#foo
end

sig {
  params(
    parent: Parent,
    ifoo: IFoo,
    empty_in_between: EmptyInBetween,
    child: Child,
    child_via_prop: ChildViaProp,
    grand_child: GrandChild
  )
  .void
}

def example(parent, ifoo, empty_in_between, child, child_via_prop, grand_child)
  parent.foo
  #      ^ hierarchy-ref: Parent#foo
  #      ^ hierarchy-ref: Child#foo
  #      ^ hierarchy-ref: ChildViaProp#foo
  ifoo.foo
  #    ^ hierarchy-ref: Child#foo
  empty_in_between.foo
  #                ^ hierarchy-ref: Parent#foo
  #                ^ hierarchy-ref: Child#foo
  #                ^ hierarchy-ref: ChildViaProp#foo
  child.foo
  #     ^ hierarchy-ref: Parent#foo
  #     ^ hierarchy-ref: Child#foo
  #     ^ hierarchy-ref: ChildViaProp#foo
  child_via_prop.foo
  #              ^ hierarchy-ref: Parent#foo
  #              ^ hierarchy-ref: Child#foo
  #              ^ hierarchy-ref: ChildViaProp#foo
  child_via_prop.foo = 1
  #              ^ hierarchy-ref: Parent#foo
  #              ^ hierarchy-ref: Child#foo
  #              ^ hierarchy-ref: ChildViaProp#foo
  grand_child.foo
  #           ^ hierarchy-ref: Parent#foo
  #           ^ hierarchy-ref: Child#foo
  #           ^ hierarchy-ref: ChildViaProp#foo
end
