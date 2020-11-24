# typed: true
# frozen_string_literal: true

class Foo
  attr_reader :foo
#              ^ apply-rename: [A] invalid: true 
end

f = Foo.new
f.foo

