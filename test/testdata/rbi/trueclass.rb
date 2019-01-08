# typed: true

T.assert_type!(true & false, T.any(FalseClass, TrueClass))
T.assert_type!(true & true, T.any(FalseClass, TrueClass))
T.assert_type!(true & BasicObject.new, T.any(FalseClass, TrueClass))
T.assert_type!(true ^ false, T.any(FalseClass, TrueClass))
T.assert_type!(true ^ true, T.any(FalseClass, TrueClass))
T.assert_type!(true ^ BasicObject.new, T.any(FalseClass, TrueClass))
T.assert_type!(true | false, TrueClass)
T.assert_type!(true | true, TrueClass)
T.assert_type!(true | BasicObject.new, TrueClass)
