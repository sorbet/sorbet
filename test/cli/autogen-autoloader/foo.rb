# typed: true

require 'byebug'
require 'my_gem'

module Foo
  module Bar
    class Quuz
      'x'
    end

    class Jazz < Quuz
      class JazBaz
        'x'
        require 'in_class'

        def honk
          require 'in_method'
        end
      end
    end
  end
end

