# typed: true
# frozen_string_literal: true

class Foo
  attr_reader :foo
#         ^ apply-rename: [A] newName: x invalid: true
  attr_writer :bar
#         ^ apply-rename: [C] newName: x invalid: true
  attr_accessor :baz
#         ^ apply-rename: [D] newName: x invalid: true
end

f = Foo.new
f.foo
#  ^ apply-rename: [B] newName: bar invalid: true
