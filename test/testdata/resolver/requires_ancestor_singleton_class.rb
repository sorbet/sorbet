# typed: true
# enable-experimental-requires-ancestor: true

module Helper
  extend T::Helpers

  requires_ancestor { T.class_of(Foo) }

  def helper
    foo
  end
end

class Foo
  class << self
    include Helper

    def foo; end

    def foo2
      helper
    end
  end
end

class Bar
  class << self
# ^^^^^ error: `T.class_of(Bar)` must inherit `T.class_of(Foo)` (required by `Helper`)
    include Helper

    def bar; end
  end
end
