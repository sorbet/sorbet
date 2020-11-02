# typed: true
# frozen_string_literal: true

module M
  class Foo
    def bar(a)
#     ^ apply-rename: [A] newName: baz
    end

    def caller
      bar(1)
    end
  end
end

f = M::Foo.new
f.bar(2)

M::Foo.new.bar 3

class Unrelated
  # this should be left unchanged
  def bar; end
end
