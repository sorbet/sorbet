# typed: true

extend T::Sig

  sig {returns(String)}
  def implicit_return_non_empty_cont_block
    puts nil
  # ^^^^^^^^ error: Expected `String` but found `NilClass` for method result type
  end
sig {returns(String)}
def double_return
  return puts nil
end

sig {params(x: T::Module[T.anything]).returns(String)}
def initialized_twice(x)
  if T.unsafe(nil)
    res = x.name
  else
    res = x.name
  end
  return res
end

sig {returns(String)}
def implicit_return_via_else
  if T.unsafe(nil)
    return 'yep'
  end
end

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
def implicit_return_without_keyword(x)
  if T.unsafe(nil)
    ""
  end
end
