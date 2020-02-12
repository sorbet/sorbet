# typed: true

require 'byebug'
require 'my_gem'

module Foo
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
