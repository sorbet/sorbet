# typed: strict

sig.returns(Integer)
def foo
  3
end
foo + :sym # error: does not match expected type
