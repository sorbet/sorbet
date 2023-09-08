# typed: true

# The problem demonstrated in this test is very similar to the one in
# no_short_circuit_type_constraint.rb. The core problem boils down to Sorbet's
# type constraints accumulating greedily.
#
# There is more context in that test. For the time being, it's not _wrong_ for
# Sorbet to infer this type, it's just not quite as narrow as you'd want it to
# be.
#
# It's also worth noting that sometimes users don't have control over how their
# nested types are created--the order may be determined by the order that
# Sorbet lubs basic block branches and glbs type tests. So sometimes the bad
# case below can happen because of no fault of the user.

module M1; end
module M2; end

module A; end
module B; end

module Main
  extend T::Sig

  # Type signature indicates that we're trying to remove the `B` from the input.
  sig do
    type_parameters(:U)
      .params(x: T.any(T.type_parameter(:U), B))
      .returns(T.type_parameter(:U))
  end
  def self.remove_the_b(x)
    case x
    when B then raise
    else x
    end
  end

  sig do
    type_parameters(:U)
      .params(x: T.all(M1, T.any(A, B), M2))
      .void
  end
  def self.test1(x)
    # Sorbet gets this type because when faced with a type like
    #
    #   X & Y  <:  A | B
    #
    # It needs to decide which comparison to do first: split the OrType:
    #   X & Y <: A   ||   X & Y <: B
    # or split the AndType:
    #   X <: A | B   ||   Y <: A | B
    # Since Sorbet's inference algorithm is greedy, one of the two choices will
    # always be wrong. In our case, `A` is actually `T.type_parameter(:U)`, and
    # the `OrType` is split first, which leads to a comparison like
    #
    #      X & Y  <:  T.type_parameter(:U)
    #   => T.all(T.all(M1, T.any(A, B)), M2)  <:  T.type_parameter(:U)
    #
    # which has the effect of recording the `B` in the type parameter, instead
    # of dropping it.
    #
    # Ideally, this would reveal `T.all(M1, A, M2)` or something similar
    inferred = remove_the_b(x)
    T.reveal_type(inferred) # error: `T.all(M1, T.any(A, B), M2)`
  end

  sig do
    type_parameters(:U)
      .params(x: T.all(M1, M2, T.any(A, B)))
      .void
  end
  def self.test2(x)
    # Written this way, things work, because we have a special case which
    # unwraps a specific case of an OrType nested inside an AndType:
    #
    #      X & (F | G)        <:  t2
    #   => (X & F) | (X & G)  <:  t2
    #
    # That is, we distribute and continue, which splits the A | B.

    inferred = remove_the_b(x)
    T.reveal_type(inferred) # error: `T.all(A, M1, M2)`
  end
end
