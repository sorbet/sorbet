# typed: false

def foo
  puts 'before'
  T::Array[T.any(Integer, String]
                 #              ^ error: unterminated "("
  puts 'after'
end
