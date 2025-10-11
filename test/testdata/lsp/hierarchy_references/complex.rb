# typed: true
class Module; include T::Sig; end

class Parent
  extend T::Helpers
  abstract!
  sig { abstract.returns(Integer) }
  def foo; end
  #   ^ hierarchy-ref: foo
end

module IFoo
  extend T::Helpers
  abstract!
  sig { abstract.returns(Integer) }
  def foo; end
  #   ^ hierarchy-ref: foo
end

class EmptyInBetween < Parent
  abstract!
end

class Child < EmptyInBetween
  include IFoo
  sig { override.returns(Integer) }
  def foo; 0; end
  #   ^ hierarchy-ref: foo
end

class ChildViaProp < Parent
  include T::Props

  prop :foo, Integer, override: :reader
  # ^ hierarchy-ref: foo
  #     ^ hierarchy-ref: foo

  sig { params(foo: Integer).void }
  def initialize(foo:)
    @foo = foo
    #^ hierarchy-ref: foo
  end

  def example
    @foo
    #^ hierarchy-ref: foo
  end
end

class GrandChild < Child
  sig { override.returns(Integer) }
  def foo; 0; end
  #   ^ hierarchy-ref: foo
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
  #      ^ hierarchy-ref: foo
  ifoo.foo
  #    ^ hierarchy-ref: foo
  empty_in_between.foo
  #                ^ hierarchy-ref: foo
  child.foo
  #     ^ hierarchy-ref: foo
  child_via_prop.foo
  #              ^ hierarchy-ref: foo
  child_via_prop.foo = 1
  #              ^ hierarchy-ref: foo
  grand_child.foo
  #           ^ hierarchy-ref: foo
end
