# typed: true
extend T::Helpers

A = T.any(Integer, String)

sig {returns(A)} # error: Constant `A` is not a class or type alias
def foo
end
