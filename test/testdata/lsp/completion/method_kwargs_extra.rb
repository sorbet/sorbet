# typed: true

class A
  def self.test(a: nil, arg: nil, argument: nil, message: nil)
  end
end

arg = 10
A.test(ar)
#      ^^  error: Method `ar` does not exist
#        ^ completion: arg: (keyword argument), argument: (keyword argument), arg, ...

A.test()
#      ^ completion: a: (keyword argument), arg: (keyword argument), argument: (keyword argument), message: (keyword argument), ...

A.test(a: 10, arg: ar, message: "message")
#                  ^^  error: Method `ar` does not exist
#                    ^ completion: argument: (keyword argument), arg, ...

A.test(a: 10, ar argument: 20)
#             ^^  error: Method `ar` does not exist
#             ^^  error: Unrecognized keyword argument `ar`
#             ^^  error: positional arg "ar" after keyword arg
#               ^ completion: arg: (keyword argument), arg, ...

A.test(a: 10, ar, argument: 20)
#           ^     error: unexpected token ","
#             ^^  error: Method `ar` does not exist
#             ^^  error: Unrecognized keyword argument `ar`
#               ^ completion: arg: (keyword argument), arg, ...

begin
  A.
  # ^ completion: test, allocate, ...
end # error: unexpected token

def bar(other: nil)
end

begin
  A.test(arg: def foo(x = bar()); end)
  #                           ^ completion: other: (keyword argument), x, ...

  # This case should not include any keyword arguments from `A.test`, as we're
  # no longer supplying arguments directly to that send.
  A.test(arg: def foo(x = a); end)
  #                       ^  error: Method `a` does not exist
  #                        ^ completion: alias, and, bar, class, ...
end

def other(message:, mes:)
  A.test(mess)
  #      ^^^^  error: Method `mess` does not exist
  #          ^ completion: message: (keyword argument), message, ...

  A.test(mes)
  #         ^ completion: message: (keyword argument), mes, message, ...

  A.test(arg: 10, mes)
  #               ^^^  error: Method `mes` does not exist
  #               ^^^  error: Unrecognized keyword argument `mes`
  #               ^^^  error: positional arg "mes" after keyword arg
  #                  ^ completion: message: (keyword argument), mes, message, ...

  A.test(arg: 10, mes message: "")
  #               ^^^  error: Method `mes` does not exist
  #               ^^^  error: Unrecognized keyword argument `mes`
  #               ^^^  error: positional arg "mes" after keyword arg
  #                  ^ completion: mes, message, ...

  A.test(arg: 10, a message: "")
  #               ^  error: Method `a` does not exist
  #               ^  error: positional arg "a" after keyword arg
  #                ^ completion: alias, and, argument: (keyword argument), message, ...

  A.test(arg: 10, mes, message: "")
  #               ^^^  error: Method `mes` does not exist
  #               ^^^  error: Unrecognized keyword argument `mes`
  #             ^      error: unexpected token ","
  #                  ^ completion: mes, message, ...

  A.test(arg: 10, a, message: "")
  #               ^  error: Method `a` does not exist
  #             ^    error: unexpected token ","
  #                ^ completion: alias, and, argument: (keyword argument), message, ...
end

class B
  def self.foo(foo_arg: nil, &blk)
    self.bar(&blk)
    #            ^ completion: blk

    self.bar(&blk)
    #        ^ completion: bar_arg: (keyword argument), blk, foo_arg, ...

    # This would be nice to fix in the future, as `bar_arg:` is already
    # supplied.
    self.bar(bar_arg: foo_arg, &blk)
    #                         ^ completion: blk, foo_arg, ...
  end

  def self.bar(bar_arg: nil, &blk)
  end
end

# This seems like a bug as the query responses are empty. Perhaps we're not
# generating send query results for call-with-block?
B.bar() do
#     ^ completion: (nothing)
end

B.bar() do
  B.foo()
  #     ^ completion: foo_arg: (keyword argument), bar, foo, ...
end
