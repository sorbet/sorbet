# typed: true
# frozen_string_literal: true

module M
  class Foo
    def bar(a=1)
    end

    def caller
      bar(1)
    end

    def x
      42
    end
  end
end

f = M::Foo.new
f.bar(f.x)
f.bar
# ^ apply-rename: [A] newName: baz placeholderText: bar
f  .  bar  (   )

M::Foo.new.bar 3

class Unrelated
  # this should be left unchanged
  def bar; end
end
