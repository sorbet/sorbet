# typed: true

  class Other::Thing
#       ^^^^^ error: File belongs to package `Producer` but defines a constant that does not match this namespace
    class Nested; end
    class << self
      class SingletonNested; end
      VALUE = 1
    end
    Param = type_member
#           ^^^^^^^^^^^ error: Method `type_member` does not exist on `T.class_of(Other::Thing)`
  end

  class Missing::Thing
#       ^^^^^^^ error: File belongs to package `Producer` but defines a constant that does not match this namespace
    class Nested; end
  end

  Other::VALUE = 1
# ^^^^^ error: File belongs to package `Producer` but defines a constant that does not match this namespace

  module Producer::Valid
    class << self
      class Nested; end
    end
  end
