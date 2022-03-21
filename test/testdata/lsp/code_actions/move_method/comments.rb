# typed: strict
# selective-apply-code-action: refactor.extract
#
# Test asserts comments and whitespaces between a sig and a method def stay in place after the refactoring

class Foo
  extend T::Sig

sig {void}
# comment between sig and def should move with the method
def bar; end
   # ^ apply-code-action: [A] Move method to a new module

sig do
  void
end # comment after sig should move with the method
def baz; end
# ^ apply-code-action: [B] Move method to a new module

sig do
  void
end


def qux
# ^ apply-code-action: [C] Move method to a new module
end
end
