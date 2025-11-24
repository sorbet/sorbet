# typed: false

def foo
  puts 'before'
  {key: T.any(Integer, String}
              #              ^ error: unexpected token tRCURLY
  puts 'after'
end
