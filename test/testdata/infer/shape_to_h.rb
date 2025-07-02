# typed: true

opts = {}
T.reveal_type(opts) # error: `{} (shape of T::Hash[T.untyped, T.untyped])`
T.reveal_type(opts.to_h) # error: `T::Hash[T.untyped, T.untyped]`

opts = {x: 1}
T.reveal_type(opts) # error: `{x: Integer(1)} (shape of T::Hash[T.untyped, T.untyped])`
T.reveal_type(opts.to_h) # error: `T::Hash[Symbol, Integer]`

opts = {x: 1, "y" => 0.0}
T.reveal_type(opts) # error: `{x: Integer(1), String("y") => Float(0.000000)} (shape of T::Hash[T.untyped, T.untyped])`
T.reveal_type(opts.to_h) # error: `T::Hash[T.any(Symbol, String), T.any(Integer, Float)]`

opts = {x: {y: 1}}
T.reveal_type(opts) # error: `{x: {y: Integer(1)}} (shape of T::Hash[T.untyped, T.untyped])`
T.reveal_type(opts.to_h) # error: `T::Hash[Symbol, {y: Integer(1)}]`

opts = {x: {y: 1}.to_h}
T.reveal_type(opts) # error: `{x: T::Hash[Symbol, Integer]} (shape of T::Hash[T.untyped, T.untyped])`
T.reveal_type(opts.to_h) # error: `T::Hash[Symbol, T::Hash[Symbol, Integer]]`
