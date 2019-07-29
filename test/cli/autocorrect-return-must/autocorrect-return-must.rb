# typed: true

extend T::Sig

sig {returns(String)}
def direct_return
  T.let("", T.nilable(String))
end

sig {params(x: T.nilable(String)).returns(String)}
def return_arg(x)
  x
end

sig {params(x: T.nilable(String)).returns(String)}
def return_uninitialized(x)
  if T.unsafe(nil)
    a = ""
  end
  a
end

sig {params(x: T.nilable(String)).returns(String)}
def implicit_return(x)
  if T.unsafe(nil)
    ""
  end
end
