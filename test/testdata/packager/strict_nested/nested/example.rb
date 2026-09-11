# typed: strict

module Root # error: `Root` belongs to package `Root`
  module Nested
    class Example
      p(Root)
      # ^^^^ error: `Root` is not imported
      p(Root::MyClass)
      # ^^^^ error: `Root` is not imported
    end

    p(Root)
    # ^^^^ error: `Root` is not imported
    p(Root::MyClass)
    # ^^^^ error: `Root` is not imported
  end
end

p(Root)
# ^^^^ error: `Root` is not imported
p(Root::MyClass)
# ^^^^ error: `Root` is not imported
