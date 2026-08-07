# typed: true

module A
  class Foo
    Y::Foo.new
    # Importing G would cause a cycle (G -> A), so no autocorrect should be shown.
    G::Foo.new
  end
end
