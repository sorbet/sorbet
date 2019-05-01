# typed: false
sig do # error: Malformed `sig`: No return type specified. Specify one with .returns()
  a&.o[]
# ^^^^^^ error: Unknown method
end
def foo
end
