# typed: true
# frozen_string_literal: true

class Foo
  attr_reader :bar
end

f = Foo.new
f.bar
# ^ apply-rename: [A] newName: baz
