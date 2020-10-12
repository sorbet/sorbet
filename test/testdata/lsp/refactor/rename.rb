# typed: true
# frozen_string_literal: true

class Foo
  class Foo
  end
end

foo = Foo.new
#     ^ apply-rename: [A] newName: Bar
