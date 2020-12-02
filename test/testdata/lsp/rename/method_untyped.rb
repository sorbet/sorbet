# typed: false

class Foo
  def foo
    puts "hi"
  end
end

Foo.new.foo
#        ^ apply-rename: [A] invalid: true
