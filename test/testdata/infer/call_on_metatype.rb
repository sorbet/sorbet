# typed: true
extend T::Sig

T.any(Integer, String).class # error: mistakes a type for a value
T.all(Kernel, Comparable).class # error: mistakes a type for a value
T.nilable(Integer).class # error: mistakes a type for a value

T::Array[Integer].===(0) # error: mistakes a type for a value

# This is a bug; there's a missing error here, hidden by https://github.com/sorbet/sorbet/issues/3426
T.noreturn.to_s

# This is also a bug; hidden by https://github.com/sorbet/sorbet/issues/3428
T.untyped.to_s

# And another one
T.self_type.to_s

# And yet another one
T.class_of(Integer).foo

# This one is weird and kind of tricky to change: https://github.com/sorbet/sorbet/issues/3427
puts (T.proc.void + 1) # error: Method `+` does not exist on `T.class_of(T.proc)`
