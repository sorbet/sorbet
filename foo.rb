# typed: true

extend T::Sig

sig {params(x: T.nilable(String), y: T.nilable(String)).returns(String)}
def foo(x, y)
  if x.nil? && y.nil?
    return 'default'
  elsif !x.nil?
    return x
  else
    return y
  end
end
