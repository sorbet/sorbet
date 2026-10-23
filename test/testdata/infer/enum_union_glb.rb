# typed: strict

# Intersections of unions of T::Enum variants with other types.
class E < T::Enum
  enums do
    A = new
    B = new
    C = new
    D = new
  end
end

class F < T::Enum
  enums do
    X = new
    Y = new
  end
end

module M; end
module Unrelated; end

class Test
  extend T::Sig

  sig { params(x: T.all(T.any(E::A, E::B, E::C), T.any(E::B, E::C, E::D))).void }
  def self.same_enum_overlap(x)
    T.reveal_type(x) # error: Revealed type: `T.any(E::B, E::C)`
  end

  sig { params(x: T.all(T.any(E::A, E::B), T.any(E::B, E::C))).void }
  def self.same_enum_single(x)
    T.reveal_type(x) # error: Revealed type: `E::B`
  end

  sig { params(x: T.all(T.any(E::A, E::B), T.any(E::C, E::D))).void }
  def self.same_enum_disjoint(x)
    T.reveal_type(x) # error: This code is unreachable
  end

  sig { params(x: T.all(T.any(E::A, E::B, E::C), T.any(E::A, E::B))).void }
  def self.same_enum_subset(x)
    T.reveal_type(x) # error: Revealed type: `T.any(E::A, E::B)`
  end

  sig { params(x: T.all(T.any(E::A, E::B), T.any(F::X, F::Y))).void }
  def self.different_enums(x)
    T.reveal_type(x) # error: This code is unreachable
  end

  sig { params(x: T.all(T.any(E::A, E::B), E::A)).void }
  def self.with_member(x)
    T.reveal_type(x) # error: Revealed type: `E::A`
  end

  sig { params(x: T.all(T.any(E::A, E::B), E::C)).void }
  def self.with_non_member(x)
    T.reveal_type(x) # error: This code is unreachable
  end

  sig { params(x: T.all(T.any(E::A, E::B), E)).void }
  def self.with_parent(x)
    T.reveal_type(x) # error: Revealed type: `T.any(E::A, E::B)`
  end

  sig { params(x: T.all(T.any(E::A, E::B), T::Enum)).void }
  def self.with_t_enum(x)
    T.reveal_type(x) # error: Revealed type: `T.any(E::A, E::B)`
  end

  sig { params(x: T.all(T.any(E::A, E::B), Object)).void }
  def self.with_object(x)
    T.reveal_type(x) # error: Revealed type: `T.any(E::A, E::B)`
  end

  sig { params(x: T.all(T.any(E::A, E::B), Kernel)).void }
  def self.with_kernel(x)
    T.reveal_type(x) # error: Revealed type: `T.any(E::A, E::B)`
  end

  sig { params(x: T.all(T.any(E::A, E::B), String)).void }
  def self.with_unrelated_class(x)
    T.reveal_type(x) # error: This code is unreachable
  end

  sig { params(x: T.all(T.any(E::A, E::B), Unrelated)).void }
  def self.with_unrelated_module(x)
    T.reveal_type(x) # error: Revealed type: `T.all(Unrelated, T.any(E::A, E::B))`
  end

  sig { params(x: T.all(T.any(E::A, E::B), T.nilable(E::B))).void }
  def self.with_nilable(x)
    T.reveal_type(x) # error: Revealed type: `E::B`
  end

  sig { params(x: T.all(T.any(E::A, E::B, E::C), T.any(E::C, F::X))).void }
  def self.with_or_of_enums(x)
    T.reveal_type(x) # error: Revealed type: `E::C`
  end
end
