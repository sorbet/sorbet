# typed: strict
# enable-experimental-rbs-comments: true

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

# @requires_ancestor: BasicObject
module RequiresAncestor1; end

# @requires_ancestor: ::BasicObject
module RequiresAncestor2; end

# @requires_ancestor: Multiple1::Multiple2
module RequiresAncestor3; end

# @unknown_annotation
class UnknownAnnotation; end
