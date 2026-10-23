# typed: true

# Behavior of unions of T::Enum variants under the type operations that flow
# typing and method calls use: merging branches (lub), narrowing (glb), and
# subtyping. These types are represented compactly, so this pins down that the
# compact representation agrees with an ordinary T.any.

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

class Test
  extend T::Sig

  # Merging control flow branches (lub)

  sig { params(ab: T.any(E::A, E::B), n: T.nilable(E), c: T::Boolean).void }
  def self.lub_with_nilable_parent(ab, n, c)
    x = c ? ab : n
    T.reveal_type(x) # error: Revealed type: `T.nilable(E)`
  end

  sig { params(ab: T.any(E::A, E::B), nab: T.nilable(T.any(E::A, E::B)), c: T::Boolean).void }
  def self.lub_with_nilable_same(ab, nab, c)
    x = c ? ab : nab
    T.reveal_type(x) # error: Revealed type: `T.nilable(T.any(E::A, E::B))`
  end

  sig { params(ab: T.any(E::A, E::B), nbc: T.nilable(T.any(E::B, E::C)), c: T::Boolean).void }
  def self.lub_with_nilable_overlap(ab, nbc, c)
    x = c ? ab : nbc
    T.reveal_type(x) # error: Revealed type: `T.nilable(T.any(E::C, E::A, E::B))`
  end

  sig { params(ab: T.any(E::A, E::B), a_and_m: T.all(E::A, M), c: T::Boolean).void }
  def self.lub_with_intersection_of_member(ab, a_and_m, c)
    x = c ? ab : a_and_m
    T.reveal_type(x) # error: Revealed type: `T.any(E::A, E::B)`
  end

  sig { params(ab: T.any(E::A, E::B), ai: T.any(E::A, Integer), c: T::Boolean).void }
  def self.lub_with_mixed_union(ab, ai, c)
    x = c ? ab : ai
    T.reveal_type(x) # error: Revealed type: `T.any(Integer, E::A, E::B)`
  end

  sig { params(ab: T.any(E::A, E::B), xy: T.any(F::X, F::Y), c: T::Boolean).void }
  def self.lub_different_enums(ab, xy, c)
    x = c ? ab : xy
    T.reveal_type(x) # error: Revealed type: `T.any(F::X, F::Y, E::A, E::B)`
  end

  sig { params(ab: T.any(E::A, E::B), c: T::Boolean).void }
  def self.lub_with_literals(ab, c)
    T.reveal_type(c ? ab : 1) # error: Revealed type: `T.any(E::A, E::B, Integer)`
    T.reveal_type(c ? ab : :sym) # error: Revealed type: `T.any(E::A, E::B, Symbol)`
    T.reveal_type([[1, ab]].flatten) # error: Revealed type: `T::Array[T.any(E::A, E::B, Integer)]`
  end

  sig { params(e: E).void }
  def self.lub_in_loop(e)
    x = T.let(E::A, E)
    [1, 2].each do |i|
      x = e if i == 1
    end
    T.reveal_type(x) # error: Revealed type: `E`
  end

  # Narrowing (glb)

  sig { params(nab: T.nilable(T.any(E::A, E::B)), abx: T.any(E::A, E::B, F::X)).void }
  def self.narrow_mixed_unions(nab, abx)
    if nab.is_a?(E)
      T.reveal_type(nab) # error: Revealed type: `T.any(E::A, E::B)`
    end
    if abx.is_a?(E)
      T.reveal_type(abx) # error: Revealed type: `T.any(E::A, E::B)`
    else
      T.reveal_type(abx) # error: Revealed type: `F::X`
    end
    if abx.is_a?(M)
      T.reveal_type(abx) # error: Revealed type: `T.all(M, T.any(E::A, E::B, F::X))`
    end
    if abx.is_a?(Integer)
      T.reveal_type(abx) # error: This code is unreachable
    end
  end

  sig { params(e: T.nilable(E)).void }
  def self.narrow_with_eqeq(e)
    if e == E::A || e == E::B
      T.reveal_type(e) # error: Revealed type: `T.any(E::A, E::B)`
      if e == E::B || e == E::C
        T.reveal_type(e) # error: Revealed type: `E::B`
      end
    elsif e.nil?
      T.reveal_type(e) # error: Revealed type: `NilClass`
    else
      T.reveal_type(e) # error: Revealed type: `T.any(E::C, E::D)`
    end
  end

  sig { params(e: E).void }
  def self.narrow_with_case(e)
    case e
    when E::A, E::B
      T.reveal_type(e) # error: Revealed type: `T.any(E::B, E::A)`
    when E::C
      T.reveal_type(e) # error: Revealed type: `E::C`
    else
      T.reveal_type(e) # error: Revealed type: `E::D`
    end
  end

  # Subtyping

  sig { params(ab: T.any(E::A, E::B), abc: T.any(E::A, E::B, E::C)).void }
  def self.subtyping(ab, abc)
    T.let(ab, T.nilable(T.any(E::A, E::B, E::C)))
    T.let(ab, T.any(E::A, Integer, E::B))
    T.let(ab, E)
    T.let(ab, T::Enum)
    T.let(ab, Comparable) # error: Argument does not have asserted type `Comparable`
    T.let(abc, T.nilable(T.any(E::A, E::B))) # error: Argument does not have asserted type `T.nilable(T.any(E::A, E::B))`
    T.let(ab, T.any(E::A, F::X)) # error: Argument does not have asserted type `T.any(E::A, F::X)`
    T.let(ab, T.all(E, M)) # error: Argument does not have asserted type `T.all(E, M)`
    T.let(E::C, T.any(E::A, E::B)) # error: Argument does not have asserted type `T.any(E::A, E::B)`
    T.let(1, T.any(E::A, E::B)) # error: Argument does not have asserted type `T.any(E::A, E::B)`
  end

  # Generics

  sig { type_parameters(:U).params(x: T.type_parameter(:U), y: T.type_parameter(:U)).returns(T.type_parameter(:U)) }
  def self.both(x, y) = x

  sig { type_parameters(:U).params(x: T.nilable(T.type_parameter(:U))).returns(T.type_parameter(:U)) }
  def self.unnil(x) = T.must(x)

  sig { params(ab: T.any(E::A, E::B), nab: T.nilable(T.any(E::A, E::B))).void }
  def self.generics(ab, nab)
    T.reveal_type(both(ab, E::C)) # error: Revealed type: `T.any(E::A, E::B, E::C)`
    T.reveal_type(both(ab, nil)) # error: Revealed type: `T.nilable(T.any(E::A, E::B))`
    T.reveal_type(unnil(nab)) # error: Revealed type: `T.any(E::A, E::B)`
    T.reveal_type([ab].map { |x| x }) # error: Revealed type: `T::Array[T.any(E::A, E::B)]`
  end

  # Method calls on a union dispatch to each variant

  sig { params(ab: T.any(E::A, E::B), nab: T.nilable(T.any(E::A, E::B))).void }
  def self.dispatch(ab, nab)
    T.reveal_type(ab.serialize) # error: Revealed type: `String`
    T.reveal_type(ab.class) # error: Revealed type: `T.any(T.class_of(E::A), T.class_of(E::B))`
    T.reveal_type(nab&.serialize) # error: Revealed type: `T.nilable(String)`
    ab.not_a_method
    #  ^^^^^^^^^^^^ error: Method `not_a_method` does not exist on `E::A` component of `T.any(E::A, E::B)`
    #  ^^^^^^^^^^^^ error: Method `not_a_method` does not exist on `E::B` component of `T.any(E::A, E::B)`
  end
end

class LowerBound
  extend T::Generic
  extend T::Sig

  Elem = type_member { {lower: T.any(E::A, E::B), upper: E} }

  sig { params(x: Elem).void }
  def put(x); end
end

LowerBound[T.any(E::A, E::B, E::C)].new.put(E::A)
LowerBound[T.any(E::A, E::B)].new.put(E::C) # error: Expected `T.any(E::A, E::B)` but found `E::C` for argument `x`

module ClassOf
  extend T::Sig

  sig { params(x: T.class_of(T.any(E::A, E::B))).void } # error: `T.class_of` must wrap each individual class type, not the outer `T.any`
  def self.class_of_union(x); end
end
