# typed: true

T.assert_type!(false & false, FalseClass)
T.assert_type!(false & true, FalseClass)
T.assert_type!(false & BasicObject.new, FalseClass)
T.assert_type!(false ^ false, T::Boolean)
T.assert_type!(false ^ true, T::Boolean)
T.assert_type!(false ^ BasicObject.new, T::Boolean)
T.assert_type!(false | false, T::Boolean)
T.assert_type!(false | true, T::Boolean)
T.assert_type!(false | BasicObject.new, T::Boolean)
