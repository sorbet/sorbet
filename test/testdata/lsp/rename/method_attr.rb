# typed: true
# frozen_string_literal: true

class Foo
  attr_reader :foo
#              ^ apply-rename: [A] invalid: true
  attr_writer :bar
#              ^ apply-rename: [C] invalid: true
  attr_accessor :baz
#                ^ apply-rename: [D] invalid: true
end

f = Foo.new
f.foo
#  ^ apply-rename: [B] newName: bar placeholderText: foo invalid: true expectedErrorMessage: Sorbet does not support renaming `attr_reader`s
