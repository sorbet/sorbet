# typed: false

def foo
  puts 'before'
  {key: T.any(Integer, String}
              #              ^ error: unterminated "("
  puts 'after'
end
