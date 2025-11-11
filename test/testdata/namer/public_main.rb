# typed: true

public def foo; end

public

def bar
  puts "Hello, world!"
end

Object.new.foo
#          ^^^ error: Non-private call to private method `foo` on `Object`
Object.new.bar
#          ^^^ error: Non-private call to private method `bar` on `Object`
