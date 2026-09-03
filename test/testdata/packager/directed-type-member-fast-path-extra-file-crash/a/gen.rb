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
    # This send is only here so that this file's usage hashes mention the name
    # `other`, which will pull this file into the fast path when a method named
    # `other` is added in another package.
    T.unsafe(nil).other
  end
end
