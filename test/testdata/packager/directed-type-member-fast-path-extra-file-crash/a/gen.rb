# typed: strict
# frozen_string_literal: true

# stratum: 0

class A::Gen
  extend T::Sig
  extend T::Generic

  Elem1 = type_member
  Elem2 = type_member
  Elem3 = type_member
  Elem4 = type_member

  sig { void }
  def takes_other
    # These sends are only here so that this file's usage hashes mention the
    # names `other` and `other2`, which will pull this file into the fast path
    # when a method with one of those names is added in another package.
    #
    # (`other2` lets us do the same thing a second time, after the slow path in
    # version 2 has reset the reopened stratum.)
    T.unsafe(nil).other
    T.unsafe(nil).other2
  end
end
