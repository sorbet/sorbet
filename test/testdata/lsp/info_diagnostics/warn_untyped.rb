# typed: strict

# warn-untyped-values: true

extend T::Sig

sig { returns(Integer) }
def foo
  my_map = T.let({ foo: 1, bar: 'baz' }, T::Hash[Symbol, T.untyped])
  my_map[:foo]
# ^^^^^^^^^^^^ info: This code is untyped
end
