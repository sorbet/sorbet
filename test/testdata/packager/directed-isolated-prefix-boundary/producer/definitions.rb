# typed: true

  class Other::Thing
# ^^^^^^^^^^^^^^^^^^ error: File belongs to package `Producer` but defines a constant that does not match this namespace
    class Nested; end
    class << self
      class SingletonNested; end
      VALUE = 1
    end
    Param = type_member # error: `type_member` may only be used on constants in the package that owns them
  end

  class Missing::Thing
# ^^^^^^^^^^^^^^^^^^^^ error: File belongs to package `Producer` but defines a constant that does not match this namespace
    class Nested; end
  end

  Other::VALUE = 1
# ^^^^^^^^^^^^ error: File belongs to package `Producer` but defines a constant that does not match this namespace

  module Producer::Valid
    class << self
      class Nested; end
    end
  end
