# typed: true

# This requires three passes: The first one resolves all the names
# that don't use ancestors; The second one resolves the superclass of
# ChildInner, and the third one resolves `A` inside `ChildInner` using
# its ancestors.
class Base
  class BaseInner
    A = 1
  end
end

class Child < Base
  class ChildInner < BaseInner
    A
  end
end
