# typed: true
extend T::Sig

T.any(Integer, String).class # error: mistakes a type for a value
T.all(Kernel, Comparable).class # error: mistakes a type for a value
T.nilable(Integer).class # error: mistakes a type for a value

T::Array[Integer].===(0) # error: mistakes a type for a value

T.noreturn.to_s # error: Call to method `to_s` on `T.noreturn` mistakes a type for a value

T.untyped.to_s # error: Call to method `to_s` on `T.untyped` mistakes a type for a value

# And another one
T.self_type.to_s

# And yet another one
T.class_of(Integer).foo

# This one is weird and kind of tricky to change: https://github.com/sorbet/sorbet/issues/3427
puts (T.proc.void + 1) # error: Method `+` does not exist on `T.class_of(T.proc)`
