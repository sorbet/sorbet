# typed: strict
T.dynamic_cast(nil, String)
T.let("foo", String)
T.assert_type!("foo", String)
T.cast("foo", String) # error: Useless cast
T.unsafe(String)
T.nilable(String)
T.proc
T.proc.params(arg0: String, arg1: Integer)
T.class_of(String)
T.noreturn
T.enum([:a, :b])

T.untyped
T.any(String) # error: Not enough arguments provided for method `T.any`.
T.all(String) # error: Not enough arguments provided for method `T.all`.
T.any(String, Integer, Symbol)
T.all(String, Integer, Symbol)


T.dynamic_cast # error: Not enough arguments provided for method `T.dynamic_cast`. Expected: `2`, got: `0`
T.let # error: Not enough arguments provided for method `T.let`. Expected: `2`, got: `0`
T.assert_type! # error: Not enough arguments provided for method `T.assert_type!`. Expected: `2`, got: `0`
T.cast # error: Not enough arguments provided for method `T.cast`. Expected: `2`, got: `0`
T.unsafe # error: Not enough arguments provided for method `T.unsafe`. Expected: `1`, got: `0`
T.nilable # error: Not enough arguments provided for method `T.nilable`. Expected: `1`, got: `0`
T.proc(String) # error: Too many arguments provided for method `T.proc`. Expected: `0`, got: `1`
T.class_of # error: Not enough arguments provided for method `T.class_of`. Expected: `1`, got: `0`
T.noreturn(String) # error: Too many arguments provided for method `T.noreturn`. Expected: `0`, got: `1`
T.enum # error: Not enough arguments provided for method `T.enum`. Expected: `1`, got: `0`

T.untyped(String) # error: Too many arguments provided for method `T.untyped`. Expected: `0`, got: `1`
T.any # error: Not enough arguments provided for method `T.any`. Expected: `2+`, got: `0`
T.all # error: Not enough arguments provided for method `T.all`. Expected: `2+`, got: `0`

T.assert_type!(false, T::Boolean)
