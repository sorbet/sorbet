# typed: strict
# enable-suggest-unsafe: true

def foo(x) # error: does not have a `sig`
  T.unsafe(nil)
end
