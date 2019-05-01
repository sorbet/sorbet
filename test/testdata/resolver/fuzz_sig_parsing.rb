# typed: false
class A; end
  sig{A.b}
#     ^^^ error: Unknown method
# ^^^^^^^^ error: Malformed `sig`: No return type specified. Specify one with .returns()
def foo; end
