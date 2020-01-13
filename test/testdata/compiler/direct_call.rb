# typed: true
# compiled: true
def foo
  return 1
end

def bar
  foo
end

puts bar
