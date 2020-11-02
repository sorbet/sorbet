# typed: true
# frozen_string_literal: true

module M
  class Foo
    def bar(a=1)
#     ^ apply-rename: [A] newName: baz
    end

    def caller
      bar(1)
    end
  end
end

f = M::Foo.new
f.bar(2)
f.bar

M::Foo.new.bar 3

class Unrelated
  # this should be left unchanged
  def bar; end
end
