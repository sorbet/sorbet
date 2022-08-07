# typed: true

def basic_example
  return
  puts(123)
  #    ^^^ error: This expression appears after an unconditional return
end

def only_once_per_method
  [1].map do |x|
    if T.unsafe(nil)
      next []
      puts(x)
      #    ^ error: This expression appears after an unconditional return
    else
      next [x]
      puts(123)
    end
  end

  return 123
  puts('hello')
end

def next_in_block
  [1].map do |x|
    if T.unsafe(nil)
      next []
    else
      next [x]
      puts(123)
      #    ^^^ error: This expression appears after an unconditional return
    end
  end
end
