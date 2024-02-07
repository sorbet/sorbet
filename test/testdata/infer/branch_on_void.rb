# typed: true
extend T::Sig

sig { void }
def returns_void
end

def example1
  x = returns_void

  if x
    #^ error: Branching on `void` value
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
    #^^^^^^ error: Branching on `void`
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
    #^^^^^^ error: Branching on `void`
    #^^^^^^ error: Branching on `void`
    puts("Hello")
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
    #^^^^^^ error: Branching on `void` value
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
    #^^^^^^ error: Branching on `void` value
    puts("Hello")
  end
end

def example6
  result =
    if [true, false].sample
      returns_void
    else
      T.unsafe(0)
    end

  # This is weird: what happens is that the simplifier makes each `if` branch
  # jump directly to the block that starts the second block condition, so
  # there's no intermediate block which joins the control flow and forces the
  # `result` type to collapse, so despite the `result` variable having type
  # `T.untyped` once we get to that block, when we check one of the two
  # branches into that if-entry-block, there's an intermediate state where
  # Sorbet can see `void`

  if result
    #^^^^^^ error: Branching on `void` value
    puts("Hello")
  end
end

def example7
  xs = Array(returns_void)
  T.reveal_type(xs) # error: `T::Array[Sorbet::Private::Static::Void]`

  if xs
    # No error for void inside applied type
    puts(xs)
  end
end
