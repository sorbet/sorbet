# typed: true
# frozen_string_literal: true

module M
  class Foo
    def bar
#     ^ apply-rename: [A] newName: baz
    end

    def caller
      bar
    end
  end
end

f = M::Foo.new
f.bar

M::Foo.new.bar

class Unrelated
  # this should be left unchanged
  def bar; end
end
