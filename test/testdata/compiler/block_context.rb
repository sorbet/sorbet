# frozen_string_literal: true
# typed: true
# compiled: true
class Foo
  def foo
    yield
  end
end

# puts self.class
Foo.new.foo {puts self.class}
Foo.new.instance_exec {puts self.class} # we don't support calling blocks that change self
