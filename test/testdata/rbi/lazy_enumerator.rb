# typed: true

T.let(T.unsafe(nil), Enumerator::Lazy[String])
#                    ^^^^^^^^^^^^^^^^^^^^^^^^ error: Use `T::Enumerator::Lazy[...]`, not `Enumerator::Lazy[...]` to declare a typed `Enumerator::Lazy`

T.assert_type!([1].lazy.collect(&:to_s), T::Enumerator::Lazy[String])

T.assert_type!([1].lazy.collect(&:to_s), T::Enumerator::Lazy[String])
T.assert_type!([1].lazy.collect, T::Enumerator::Lazy[Integer])
T.assert_type!(['abcd'].lazy.collect_concat{|s| return [1, 2] }, T::Enumerator::Lazy[Integer])
T.assert_type!([1].lazy.drop(4), T::Enumerator::Lazy[Integer])
T.assert_type!([1].lazy.drop_while(&:even?), T::Enumerator::Lazy[Integer])
T.assert_type!([1].lazy.drop_while, T::Enumerator::Lazy[Integer])
T.assert_type!([1].lazy.find_all(&:even?), T::Enumerator::Lazy[Integer])
T.assert_type!([1].lazy.find_all, T::Enumerator::Lazy[Integer])

## Generics are not sufficiently robust to type this function yet
T.assert_type!([1].lazy.flat_map, T::Enumerator::Lazy[Integer])
T.assert_type!([1].lazy.flat_map{|i| [i + 1]}, T::Enumerator::Lazy[Integer])

T.assert_type!([1].lazy.grep(1..10), T::Enumerator::Lazy[Integer])
T.assert_type!([1].lazy.grep(1..10, &:to_s), T::Enumerator::Lazy[String])
T.assert_type!([1].lazy.lazy, T::Enumerator::Lazy[Integer])
T.assert_type!([1].lazy.map(&:to_s), T::Enumerator::Lazy[String])
T.assert_type!([1].lazy.map, T::Enumerator::Lazy[Integer])
T.assert_type!([1].lazy.reject(&:even?), T::Enumerator::Lazy[Integer])
T.assert_type!([1].lazy.reject, T::Enumerator::Lazy[Integer])
T.assert_type!([1].lazy.select(&:even?), T::Enumerator::Lazy[Integer])
T.assert_type!([1].lazy.select, T::Enumerator::Lazy[Integer])
T.assert_type!([1].lazy.take(10), T::Enumerator::Lazy[Integer])
T.assert_type!([1].lazy.take_while{|i| i < 10 }, T::Enumerator::Lazy[Integer])
T.assert_type!([1].lazy.take_while, T::Enumerator::Lazy[Integer])
T.assert_type!([1].lazy.filter_map {|i| i.to_s if i.even? }, T::Enumerator::Lazy[String])
