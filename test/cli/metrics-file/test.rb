# typed: true

class Foo
  def foo
    puts "foo"
  end
end

class Bar
  delegate :foo, to: :foo
end

Foo.new.foo
Bar.new.foo
