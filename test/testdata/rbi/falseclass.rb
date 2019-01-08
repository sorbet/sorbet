# typed: true

T.assert_type!(false & false, FalseClass)
T.assert_type!(false & true, FalseClass)
T.assert_type!(false & BasicObject.new, FalseClass)
T.assert_type!(false ^ false, T.any(FalseClass, TrueClass))
T.assert_type!(false ^ true, T.any(FalseClass, TrueClass))
T.assert_type!(false ^ BasicObject.new, T.any(FalseClass, TrueClass))
T.assert_type!(false | false, T.any(FalseClass, TrueClass))
T.assert_type!(false | true, T.any(FalseClass, TrueClass))
T.assert_type!(false | BasicObject.new, T.any(FalseClass, TrueClass))
