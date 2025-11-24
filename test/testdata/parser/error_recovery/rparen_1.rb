# typed: false

def foo
  puts 'before'
  T::Array[T.any(Integer, String]
                 #              ^ error: unexpected token "]"
  puts 'after'
end
