# typed: strict
extend T::Sig

sig {returns(Class)}
#            ^^^^^ error: Generic class without type arguments `Class`
def example
  Integer
end

sig {returns(Module)}
def example2
  Integer
end
