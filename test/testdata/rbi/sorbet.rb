# typed: true

Sorbet.sig.returns(String)
def foo
  2 # error: Returning value that does not conform to method result
end
