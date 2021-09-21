# typed: strict

extend T::Sig

# We treat empty shapes and tuples as containers of untyped because it is so
# common for people to mutate them, expanding their type.
#
# We could imagine a world where at `# typed: strict` users are required to
# type their empty array and hash literals. But for now, we treat them as
# containers of `T.untyped` implicitly

sig {void}
def shape_test1
  T.reveal_type({}) # error: Revealed type: `{} (shape of T::Hash[T.untyped, T.untyped])`
  T.reveal_type({}[:foo]) # error: Revealed type: `T.untyped`

  {}.fetch(:foo)
  puts 'should be reachable'
end

sig {void}
def shape_test2
  xs = {}
  1.times do
    xs[:foo] = 1
  end

  T.reveal_type(xs) # error: Revealed type: `{} (shape of T::Hash[T.untyped, T.untyped])`
end

sig {void}
def tuple_test1
  T.reveal_type([]) # error: Revealed type: `[] (0-tuple)`
  T.reveal_type([][0]) # error: Revealed type: `NilClass`

  [].fetch(0)
  puts 'should be reachable'
end

sig {void}
def tuple_test2
  xs = []
  1.times do
    xs[0] = 1
  end

  T.reveal_type(xs) # error: Revealed type: `[] (0-tuple)`
end
