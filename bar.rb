# typed: true

extend T::Sig

sig {params(x: T.nilable(String), y: T.nilable(String)).returns(String)}
def bar(x, y)
  if !x && !y
    return 'default'
  elsif x
    return x
  else
    return y
  end
end
