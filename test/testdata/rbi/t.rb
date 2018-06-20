# typed: strict
T.dynamic_cast(nil, String)
T.let("foo", String)
T.assert_type!("foo", String)
T.cast("foo", String) # error: Useless cast
T.unsafe(String)
T.nilable(String)
T.proc
T.proc(arg0: String, arg1: Integer)
T.class_of(String)
T.noreturn
T.enum([:a, :b])

T.untyped
T.any(String, Integer, Symbol)
T.all(String, Integer, Symbol)


T.dynamic_cast # error: Not enough arguments provided for method `dynamic_cast`. Expected: `2`, got: `0`
T.let # error: Not enough arguments provided for method `let`. Expected: `2`, got: `0`
T.assert_type! # error: Not enough arguments provided for method `assert_type!`. Expected: `2`, got: `0`
T.cast # error: Not enough arguments provided for method `cast`. Expected: `2`, got: `0`
T.unsafe # error: Not enough arguments provided for method `unsafe`. Expected: `1`, got: `0`
T.nilable # error: Not enough arguments provided for method `nilable`. Expected: `1`, got: `0`
T.proc(String) # error: Too many arguments provided for method `proc`. Expected: `1`, got: `1`
T.class_of # error: Not enough arguments provided for method `class_of`. Expected: `1`, got: `0`
T.noreturn(String) # error: Too many arguments provided for method `noreturn`. Expected: `0`, got: `1`
T.enum # error: Not enough arguments provided for method `enum`. Expected: `1`, got: `0`

T.untyped(String) # error: Too many arguments provided for method `untyped`. Expected: `0`, got: `1`
T.any # error: Not enough arguments provided for method `any`. Expected: `2`, got: `0`
T.all # error: Not enough arguments provided for method `all`. Expected: `2`, got: `0`
