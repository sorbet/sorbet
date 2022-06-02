# typed: strict

module Root
  module Nested
    class Example
      p(Root)
      # ^^^^ error: resolves but its package is not imported
      p(Root::MyClass)
      # ^^^^^^^^^^^^^ error: resolves but its package is not imported
    end

    p(Root)
    # ^^^^ error: resolves but its package is not imported
    p(Root::MyClass)
    # ^^^^^^^^^^^^^ error: resolves but its package is not imported
  end
end

p(Root)
# ^^^^ error: resolves but its package is not imported
p(Root::MyClass)
# ^^^^^^^^^^^^^ error: resolves but its package is not imported
