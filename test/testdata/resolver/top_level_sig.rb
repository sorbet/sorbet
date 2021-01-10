# typed: true
extend T::Sig

sig {returns(Integer)}
def foo
  3
end
foo + :sym # error: Expected `Integer` but found `Symbol(:sym)` for argument `arg0`
