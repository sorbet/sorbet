# typed: true
# frozen_string_literal: true

FOO = 1
# ^ apply-rename: [C] newName: BAZ placeholderText: FOO

class Foo
#     ^ apply-rename: [B] newName: foo placeholderText: Foo invalid: true
  class Foo
  end
end

foo = Foo.new
#     ^ apply-rename: [A] newName: Bar placeholderText: Foo
