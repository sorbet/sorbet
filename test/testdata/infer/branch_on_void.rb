# typed: true
extend T::Sig

sig { void }
def returns_void
end

def example1
  x = returns_void

  if x
    #^ error: Branching on `void` value, which is always truthy
    puts("Hello")
  end
end

def example2
  result =
    if [true, false].sample
      returns_void
    else
      nil
    end

  if result
    #^^^^^^ error: Branching on `void`, which might be truthy
    puts("Hello")
  end
end

def example3
  result =
    if [true, false].sample
      returns_void
    else
      returns_void
    end

  if result
    #^^^^^^ error: Branching on `void`, which might be truthy
    puts("Hello")
    T.reveal_type(result)
  end
end

def example4
  result =
    if [true, false].sample
      returns_void
    else
      nil
    end

  puts('force control flow to join')

  if result
    puts("Hello")
  end
end

def example5
  result =
    if [true, false].sample
      returns_void
    else
      0
    end

  puts('force control flow to join')

  if result
    puts("Hello")
  end
end
