# typed: true
extend T::Sig

T.any(Integer, String).class # error: mistakes a type for a value
T.all(Kernel, Comparable).class # error: mistakes a type for a value
T.nilable(Integer).class # error: mistakes a type for a value

T::Array[Integer].===(0) # error: mistakes a type for a value

T.noreturn.to_s # error: Call to method `to_s` on `T.noreturn` mistakes a type for a value

T.untyped.to_s # error: Call to method `to_s` on `T.untyped` mistakes a type for a value

# And another one
T.self_type.to_s # error: Call to method `to_s` on `T.self_type (of T.class_of(<root>))` mistakes a type for a value

# And yet another one
T.class_of(Integer).to_s # error: Call to method `to_s` on `T.class_of(Integer)` mistakes a type for a value

puts (T.proc.void + 1) # error: Call to method `+` on `T.proc.void` mistakes a type for a value

# Edge cases. Do anything but crash.
T.class_of.foo # error: Not enough arguments
T.class_of(Integer, String).foo # error: Too many arguments

# T::Types::Base methods
x = nil
y = T.nilable(Integer).valid?(x)
T.reveal_type(y) # error: `T.untyped`
y = T.nilable(Integer).recursively_valid?(x)
T.reveal_type(y) # error: `T.untyped`
y = T.nilable(Integer).subtype_of?(x)
T.reveal_type(y) # error: `T.untyped`
y = T.nilable(Integer).describe_obj(x)
T.reveal_type(y) # error: `T.untyped`
y = T.nilable(Integer).error_message_for_obj(x)
T.reveal_type(y) # error: `T.untyped`
y = T.nilable(Integer).error_message_for_obj_recursive(x)
T.reveal_type(y) # error: `T.untyped`
y = T.nilable(Integer).validate!(x)
T.reveal_type(y) # error: `T.untyped`

[T::Array[Integer]].first.to_s # error: Call to method `to_s` on `T::Array[Integer]` mistakes a type for a value
