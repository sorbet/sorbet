# typed: true
extend T::Sig

# Not all of the behaviors in this file are necessarily desired.
# This test serves more as a mirror to tell you how the change you've made to
# TypeConstraints works on some examples you might have not thought of.
# If you make a change that causes this test to fail, you and your reviewer
# will have to decide on your own whether that change is an improvement or a
# regression.

untyped = T.unsafe(nil)
integer = T.let(0, Integer)
string = T.let('', String)
int_or_string = T.let(0, T.any(Integer, String))
string_or_int = T.let(0, T.any(String, Integer))

sig do
  type_parameters(:U, :V)
  .params(
    u_or_v: T.any(T.type_parameter(:U), T.type_parameter(:V)),
    u: T.type_parameter(:U),
    v: T.type_parameter(:V),
  )
    .returns([T.type_parameter(:U), T.type_parameter(:V)])
end
def u_or_v__u__v(u_or_v, u, v)
  T.reveal_type(u_or_v) # error: `T.any(T.type_parameter(:U) (of Object#u_or_v__u__v), T.type_parameter(:V) (of Object#u_or_v__u__v))`
  T.reveal_type(u) # error: `T.type_parameter(:U) (of Object#u_or_v__u__v)`
  T.reveal_type(v) # error: `T.type_parameter(:V) (of Object#u_or_v__u__v)`
  [u, v]
end

sig do
  type_parameters(:U, :V)
  .params(
    u: T.type_parameter(:U),
    u_or_v: T.any(T.type_parameter(:U), T.type_parameter(:V)),
    v: T.type_parameter(:V),
  )
    .returns([T.type_parameter(:U), T.type_parameter(:V)])
end
def u__u_or_v__v(u, u_or_v, v)
  T.reveal_type(u_or_v) # error: `T.any(T.type_parameter(:U) (of Object#u__u_or_v__v), T.type_parameter(:V) (of Object#u__u_or_v__v))`
  T.reveal_type(u) # error: `T.type_parameter(:U) (of Object#u__u_or_v__v)`
  T.reveal_type(v) # error: `T.type_parameter(:V) (of Object#u__u_or_v__v)`
  [u, v]
end

sig do
  type_parameters(:U, :V)
  .params(
    u: T.type_parameter(:U),
    v: T.type_parameter(:V),
    u_or_v: T.any(T.type_parameter(:U), T.type_parameter(:V)),
  )
    .returns([T.type_parameter(:U), T.type_parameter(:V)])
end
def u__v__u_or_v(u, v, u_or_v)
  T.reveal_type(u_or_v) # error: `T.any(T.type_parameter(:U) (of Object#u__v__u_or_v), T.type_parameter(:V) (of Object#u__v__u_or_v))`
  T.reveal_type(u) # error: `T.type_parameter(:U) (of Object#u__v__u_or_v)`
  T.reveal_type(v) # error: `T.type_parameter(:V) (of Object#u__v__u_or_v)`
  [u, v]
end


x = u_or_v__u__v(integer, integer, string)
T.reveal_type(x) # error: `[Integer, String] (2-tuple)`
x = u__u_or_v__v(integer, integer, string)
T.reveal_type(x) # error: `[Integer, String] (2-tuple)`
x = u__v__u_or_v(integer, string, integer)
T.reveal_type(x) # error: `[Integer, String] (2-tuple)`

x = u_or_v__u__v(untyped, integer, string)
T.reveal_type(x) # error: `[T.untyped, String] (2-tuple)`
x = u__u_or_v__v(integer, untyped, string)
T.reveal_type(x) # error: `[T.untyped, String] (2-tuple)`
x = u__v__u_or_v(integer, string, untyped)
T.reveal_type(x) # error: `[T.untyped, String] (2-tuple)`

x = u_or_v__u__v(untyped, false, string)
T.reveal_type(x) # error: `[T.untyped, String] (2-tuple)`
x = u__u_or_v__v(false, untyped, string)
T.reveal_type(x) # error: `[T.untyped, String] (2-tuple)`
x = u__v__u_or_v(false, string, untyped)
T.reveal_type(x) # error: `[T.untyped, String] (2-tuple)`

x = u_or_v__u__v(untyped, integer, false)
T.reveal_type(x) # error: `[T.untyped, FalseClass] (2-tuple)`
x = u__u_or_v__v(integer, untyped, false)
T.reveal_type(x) # error: `[T.untyped, FalseClass] (2-tuple)`
x = u__v__u_or_v(integer, false, untyped)
T.reveal_type(x) # error: `[T.untyped, FalseClass] (2-tuple)`

x = u_or_v__u__v(int_or_string, integer, false)
T.reveal_type(x) # error: `[T.any(Integer, String), FalseClass] (2-tuple)`
x = u__u_or_v__v(integer, int_or_string, false)
T.reveal_type(x) # error: `[T.any(Integer, String), FalseClass] (2-tuple)`
x = u__v__u_or_v(integer, false, int_or_string)
T.reveal_type(x) # error: `[T.any(Integer, String), FalseClass] (2-tuple)`

x = u_or_v__u__v(string_or_int, integer, false)
T.reveal_type(x) # error: `[T.any(String, Integer), FalseClass] (2-tuple)`
x = u__u_or_v__v(integer, string_or_int, false)
T.reveal_type(x) # error: `[T.any(Integer, String), FalseClass] (2-tuple)`
x = u__v__u_or_v(integer, false, string_or_int)
T.reveal_type(x) # error: `[T.any(Integer, String), FalseClass] (2-tuple)`

x = u_or_v__u__v(int_or_string, false, false)
T.reveal_type(x) # error: `[T.any(Integer, String, FalseClass), FalseClass] (2-tuple)`
x = u__u_or_v__v(false, int_or_string, false)
T.reveal_type(x) # error: `[T.any(FalseClass, Integer, String), FalseClass] (2-tuple)`
x = u__v__u_or_v(false, false, int_or_string)
T.reveal_type(x) # error: `[T.any(FalseClass, Integer, String), FalseClass] (2-tuple)`

