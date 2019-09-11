# typed: true
class Vector; end
class Matrix; end

module Level1
  class Matrix; end
  module Foo; end
  module Level2
    module Foo; end
  end
end

# Resolving loaded references and shadowed loaded references
Vector
Matrix
module Level1
  R1 = Matrix
  R2 = Vector
end

# Resolving Foo from the top-level
Level1::Foo
Level1::Level2::Foo
Foo # error: Unable to resolve constant

# Resolving Foo in all nestings
module Level1
  R3 = Foo
end
module Level1::Level2
  R4 = Foo
end
module Level1
  module Level2
    R5 = Foo
  end
end
