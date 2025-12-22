# typed: true

# Tests that ClassDef expressions produce values in the CFG.
# The <static-init> method should return T.class_of(Foo), not NilClass.

class A
  X = class Foo
  end
end
