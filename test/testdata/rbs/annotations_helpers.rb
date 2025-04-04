# typed: strict
# enable-experimental-rbs-signatures: true

# @abstract
class Abstract1; end

# @abstract
class Abstract2
  extend T::Helpers
end

# @abstract
class Abstract3
  extend ::T::Helpers
end

# @abstract
# @abstract
class Abstract4
  extend ::T::Helpers
  abstract!
end

# @abstract
module Abstract5; end

module Abstract6
  # @abstract
  class << self; end
end

# @interface
#  ^^^^^^^^^ error: Classes can't be interfaces. Use `abstract!` instead of `interface!`
class Interface1; end

# @interface
module Interface2
  extend ::T::Helpers
  interface!
end

module Interface3
  # @interface
  #  ^^^^^^^^^ error: Classes can't be interfaces. Use `abstract!` instead of `interface!`
  class << self; end
end

# @final
class Final1
  # @final
  module Final2
    # @final
    class << self; end
  end
end

# @sealed
class Sealed1
  # @sealed
  module Sealed2
    # @sealed
    class << self; end
  end
end

# @abstract
# @final
class Multiple1
  # @interface
  # @final
  module Multiple2
    # @abstract
    # @final
    class << self; end
  end
end

# @mixes_in_class_methods:
#                         ^ error: Failed to parse RBS type (unexpected token for simple type)
module MixesInClassMethods1; end

# @mixes_in_class_methods: _
#                          ^ error: Failed to parse RBS type (unexpected token for simple type)
module MixesInClassMethods2; end

# @mixes_in_class_methods: untyped
#                          ^^^^^^^ error: Expected `Module` but found `Runtime object representing type: T.untyped` for argument `mod`
#  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Argument to `mixes_in_class_methods` must be statically resolvable to a module
module MixesInClassMethods3; end

# @mixes_in_class_methods: ClassMethods
module ClassMethods1
  module ClassMethods
  end
end

# @mixes_in_class_methods: ClassMethods1::ClassMethods
module ClassMethods2; end

# @mixes_in_class_methods: ::ClassMethods1::ClassMethods
module ClassMethods2; end

# @requires_ancestor: BasicObject
module RequiresAncestor1; end
