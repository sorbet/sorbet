# typed: true

require 'byebug'
require 'my_gem'

module Foo

  # :Foo doesn't define behavior therefore this constant should get its own
  # autoloader file.
  TOP_LEVEL_CONST = some_method

  module Bar
    class Quuz
      p 'x'
    end

    class Jazz < Quuz
      class JazBaz
        p 'x'
        require 'in_class'

        def honk
          require 'in_method'
        end
      end
    end
  end
end

module DontInclude
  p 1
end
