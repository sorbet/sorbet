# typed: strict

# initializing with array
T.assert_type!(T::Array[String].new(['a', 'b', 'c']), T::Array[String])
#  initializing with size
T.assert_type!(T::Array[T.nilable(String)].new(3), T::Array[T.nilable(String)])
#  initializing with size and initial  element
T.assert_type!(T::Array[String].new(3, 'a'), T::Array[String])

# no arg, no block
T.assert_type!(T::Array[Float].new.sum, T.any(Float, Integer))
T.assert_type!([Rational(1, 2)].sum, Rational)
# block, no arg
T.assert_type!([1.0].sum {|f| f.to_i}, Integer)
T.assert_type!([].sum {|f| Rational(1, 2)}, T.any(Rational, Integer))
# arg, no block
T.assert_type!([1.0].sum(1), T.any(Float, Integer))
T.assert_type!(T::Array[Complex].new.sum(1.0), T.any(Complex, Float))
T.assert_type!([Rational(1, 2)].sum(3), T.any(Rational, Integer))
# arg and block
T.assert_type!([1].sum(1.0) {|t| t.to_f}, Float)
# this should raise an error but does not:
T.assert_type!(T::Array[Float].new.sum('a') {|t| 1.0}, T.any(Float, String))

# Zip can zip up nils if arrays are diff lengths
T.assert_type!([1,2].zip([2]), T::Array[[Integer, T.nilable(Integer)]])

[1, 2] - [1, nil]

# array permutation/combinations with no block
T.assert_type!([1,2].permutation, T::Enumerator[T::Array[Integer]])
T.assert_type!([1,2].repeated_permutation(1), T::Enumerator[T::Array[Integer]])
T.assert_type!([1,2].combination(1), T::Enumerator[T::Array[Integer]])
T.assert_type!([1,2].repeated_combination(1), T::Enumerator[T::Array[Integer]])

# array permutation/combinations with a block
T.assert_type!([1,2].permutation {}, T::Array[Integer])
T.assert_type!([1,2].repeated_permutation(1) {}, T::Array[Integer])
T.assert_type!([1,2].combination(1) {}, T::Array[Integer])
T.assert_type!([1,2].repeated_combination(1) {}, T::Array[Integer])

# assignments
arr = [1, 2, 3]
T.assert_type!(arr[0] = 1, Integer)
T.assert_type!(arr[1..2] = 3, Integer)
T.assert_type!(arr[1..2] = [100, 200], T::Array[Integer])

# errors

T::Array[Float].new.sum.nan? # error: Method `nan?` does not exist on `Integer` component of `T.any(Integer, Float)`

# bsearch
T.reveal_type([1,2].bsearch {|x| x == 1}) # error: Revealed type: `T.nilable(Integer)`
T.reveal_type([1,2].bsearch {|x| x}) # error: Revealed type: `T.nilable(Integer)`
T.reveal_type(['a','b','c'].bsearch {|x| x == 'a'}) # error: Revealed type: `T.nilable(String)`

# bsearch_index
T.reveal_type(['a','b','c'].bsearch_index {|x| x == 'a'}) # error: Revealed type: `T.nilable(Integer)`

T.assert_type!([1, 2].to_set { |x| x + 1 }, T::Set[T.untyped])
T.assert_type!([1, 2].to_set, T::Set[T.untyped])

# intersecting
arr = [1, 2, 3]
T.assert_type!(arr.intersection([3, 5]), T::Array[Integer])
T.assert_type!(arr.intersect?([2, 7]), T::Boolean)

T.reveal_type(arr.fetch(0, -1)) # error: Revealed type: `Integer`
T.reveal_type(arr.fetch(0) { 1 }) # error: Revealed type: `Integer`

T.reveal_type(arr.fetch(0, 'error')) # error: Revealed type: `T.any(Integer, String)`
T.reveal_type(arr.fetch(0) { 'error' }) # error: Revealed type: `T.any(Integer, String)`
