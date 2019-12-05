# typed: true


module Scope1
  class A
    def foobar
      # ^^^^^^ symbol-search: "foobar", rank=0
    end
  end
end

# // Don't penalize prefix matches
module Scope2
  module F
    module O
      module O
        module Bar
          #    ^^^ symbol-search: "foobar", rank=10
        end
        module B
          module Ar
            #    ^^ symbol-search: "foobar", rank=10
          end
          module A
            module R
              #    ^ symbol-search: "foobar", rank=10
            end
          end
        end
      end
    end
  end
end

# // Don't let greedy prefix matching score these matches poorly
module Scope3
  module Fo
    module Foo
      module Ba
        module Bar
          #    ^^^ symbol-search: "foobar", rank=10
          def ar
            # ^^ symbol-search: "foobar", rank=10
          end
          def bar
            # ^^^ symbol-search: "foobar", rank=10
          end
          def obar
            # ^^^^ symbol-search: "foobar", rank=10
          end
        end
        def bar
          # ^^^ symbol-search: "foobar", rank=10
        end
      end
      module B
        module Ar
          #    ^^ symbol-search: "foobar", rank=10
        end
      end
    end
  end
end
