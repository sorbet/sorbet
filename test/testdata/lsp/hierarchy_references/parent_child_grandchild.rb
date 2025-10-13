# typed: true
class Module; include T::Sig; end

class Parent
  extend T::Helpers
  abstract!
  sig { abstract.void }
  def foo; end
  #   ^ hierarchy-ref-set: foo
end

class Child < Parent
  sig { override.void }
  def foo; end
  #   ^ hierarchy-ref-set: foo
end

class GrandChild < Child
  sig { override.void }
  def foo; end
  #   ^ hierarchy-ref-set: foo
end

sig {
  params(
    parent: Parent,
  )
  .void
}
def example(parent)
  parent.foo
  #      ^ hierarchy-ref-set: foo
end
