# typed: strict

class Other
end

# Test scoping when resolving ancestors. The superclass is typeAlias in
# the outer scope and should bind to ::Outer. Mixins are typeAlias
# inside the class, and should bind to the two inner modules.

class Test < Other
  module Mixin
  end
  module Other
  end

  include Mixin
  include Other
end

T.assert_type!(Test.new, Test::Mixin)
T.assert_type!(Test.new, Test::Other)
T.assert_type!(Test.new, ::Other)
