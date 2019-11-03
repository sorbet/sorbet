# typed: true

T.assert_type!([1].lazy.collect(&:to_s), Enumerator::Lazy[String])
T.assert_type!([1].lazy.collect, Enumerator::Lazy[String])

T.assert_type!(['abcd'].lazy.collect_concat{|s| s.chars.map(&:ord) }, Enumerator::Lazy[Integer])

T.assert_type!([1].lazy.drop(4), Enumerator::Lazy[Integer])
T.assert_type!([1].lazy.drop_while(&:even?), Enumerator::Lazy[Integer])
