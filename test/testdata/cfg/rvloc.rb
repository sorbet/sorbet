# typed: true
extend T::Sig

sig {returns(String)}
def ex1
  unanalyzable = T.unsafe(nil)
  if unanalyzable
    ''
  end
end

sig {returns(T::Boolean)}
def ex2
  puts "hello, world!"
  puts "hello, world!"
  puts "hello, world!"
  puts "hello, world!"
  puts "hello, world!"
  puts "hello, world!"
  value = "foobaR"
  return if value !~

  true
end

sig {returns(String)}
def ex3
  return true if T.unsafe(nil)
  return false if T.unsafe(nil)
  return 0 if T.unsafe(nil)
  return :symbol if T.unsafe(nil)
  ''
end

sig {returns(String)}
def ex4
  if T.unsafe(nil)
    true
  elsif T.unsafe(nil)
    false
  elsif T.unsafe(nil)
    0
  elsif T.unsafe(nil)
    :symbol
  else
    ''
  end
end
