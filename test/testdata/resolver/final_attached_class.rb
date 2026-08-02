# typed: true

# Regression test for https://github.com/sorbet/sorbet/issues/9460
#
# `T.attached_class` is legal here: `make` is a singleton method. The only
# real problem is the missing `sig(:final)`. Fixing that error should not
# require also fixing a `T.attached_class` error that shouldn't exist.

class Parent
  extend T::Sig, T::Helpers
  final!

  sig { returns(T.attached_class) }
  def self.make # error: `Parent` was declared as final but its method `make` was not declared as final
    raise
  end
end

# Same bug, but with two non-final methods in the class. The final-method
# validator visits each offending method and re-parses the preceding `sig`
# to build the "Replace with `sig(:final)`" autocorrect. Before the fix,
# each visit re-parsed the `sig { returns(T.attached_class) }` above `make`,
# so the bogus error was emitted once per non-final method in the class.
class MultipleNonFinalMethods
  extend T::Sig, T::Helpers
  final!

  sig { returns(T.attached_class) }
  def self.make # error: `MultipleNonFinalMethods` was declared as final but its method `make` was not declared as final
    raise
  end

  sig { void }
  def other; end # error: `MultipleNonFinalMethods` was declared as final but its method `other` was not declared as final
end
