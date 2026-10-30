# frozen_string_literal: true
# typed: false

# As neither of these reference constants, it should be fine to forward-declare them.
module A
  module B
  end
end

# There is no scope for this definition, so it's fine.
module A
end

# There is a scope here, but it resolves at the top-level, so it's fine.
module A::B
end

# Again, there's no constant references in the body of this declaration, so it's fine
module A::B
  module C
  end
end

# This is an error because the scope on `C::D` would cause constant lookup to occur.
module A::B # error: Package `A::B::C` may not open `A::B`
  module C::D
  end
end
