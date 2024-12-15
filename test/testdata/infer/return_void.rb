# typed: true
extend T::Sig

sig { void }
def example
  1.times do
    return 42
    #      ^^ error: Explicitly returning a value from a `void` method
  end

  if [true, false].sample
    return 17
    #      ^^ error: Explicitly returning a value from a `void` method
  end

  if [true, false].sample
    return # ok
  end

  x = 95 # ok
end
