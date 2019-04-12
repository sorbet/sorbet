# typed: true

T.assert_type!(true & false, T::Boolean)
T.assert_type!(true & true, T::Boolean)
T.assert_type!(true & BasicObject.new, T::Boolean)
T.assert_type!(true ^ false, T::Boolean)
T.assert_type!(true ^ true, T::Boolean)
T.assert_type!(true ^ BasicObject.new, T::Boolean)
T.assert_type!(true | false, TrueClass)
T.assert_type!(true | true, TrueClass)
T.assert_type!(true | BasicObject.new, TrueClass)
