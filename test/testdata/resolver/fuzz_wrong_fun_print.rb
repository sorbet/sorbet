# typed: false
sig do # error: Malformed `sig`: No return type specified. Specify one with .returns()
  a&.o[]
# ^^^^ error: Method does not exist on `Sorbet::Private::Builder`
# ^^^^^^ error: Method `[]` does not exist on `Sorbet::Private::Builder`
end
def foo
end
