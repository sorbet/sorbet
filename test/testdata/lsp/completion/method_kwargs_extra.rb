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
#                    ^ completion: arg: (keyword argument), argument: (keyword argument), arg, ...

A.test(a: 10, ar argument: 20)
#             ^^  error: Method `ar` does not exist
#             ^^  error: Unrecognized keyword argument `ar`
#             ^^  error: positional arg "ar" after keyword arg
#               ^ completion: arg: (keyword argument), argument: (keyword argument), arg, ...

A.test(a: 10, ar, argument: 20)
#           ^     error: unexpected token ","
#             ^^  error: Method `ar` does not exist
#             ^^  error: Unrecognized keyword argument `ar`
#               ^ completion: arg: (keyword argument), argument: (keyword argument), arg, ...

begin
  A.
#   ^ completion: test, allocate, ...
end # error: unexpected token

def bar(other: nil)
end

A.test(arg: def foo(x = bar()); end)
#                           ^ completion: other: (keyword argument), ...

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
  #                  ^ completion: message: (keyword argument), mes, message, ...

  A.test(arg: 10, mes, message: "")
  #               ^^^  error: Method `mes` does not exist
  #               ^^^  error: Unrecognized keyword argument `mes`
  #             ^      error: unexpected token ","
  #                  ^ completion: message: (keyword argument), mes, message, ...
end
