# typed: true

extend T::Sig

sig {returns(String)}
def direct_return
  T.let("", T.nilable(String))
end
