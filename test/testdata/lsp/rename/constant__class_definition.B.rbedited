# typed: true
# frozen_string_literal: true

FOO = 1
# ^ apply-rename: [C] newName: BAZ

class Foo
#     ^ apply-rename: [B] newName: foo invalid: true
  class Foo
  end
end

foo = Foo.new
#     ^ apply-rename: [A] newName: Bar
