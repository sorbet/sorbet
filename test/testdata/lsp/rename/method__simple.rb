# typed: true
# frozen_string_literal: true

class Foo
  def bar
#     ^ apply-rename: [A] newName: baz
  end
end

f = Foo.new
f.bar
