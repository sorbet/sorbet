# typed: true
class A
end

class << A # error: `class << EXPRESSION` is only supported for `class << self`
    def a
        'a'
    end
end

class B
    class << self
        def b
            'b'
        end
    end
end

$c = Object.new
class << $c # error: `class << EXPRESSION` is only supported for `class << self`
    def c
        "c"
    end
end

class D
    class << self
        class << self
            def d
                'd'
            end
        end
    end
end

class E
    class << self
        def wrapper
            def e
                'e'
            end
        end
    end
    wrapper
end

class F
  class << self
        def initialize
          @f = T.let(0, Integer)
        end

        extend T::Sig

        sig {params(f: Integer).returns(Integer)}
        attr_writer :f
    end
end

class G
    def wrapper
        class << self
            def inner
                T.reveal_type(self) # error: type: `T.class_of(G)`
            end
        end
        inner # error: Method `inner` does not exist on `G`
    end
    def self.g
        "g"
    end
end

class H
    class << self
        class H2
            def self.h
                "h"
            end
        end
    end
end

def main
    puts A.a # error: Method `a` does not exist on `T.class_of(A)`
    puts B.b
    puts $c.c
    puts D.singleton_class.d
    puts E.e
    puts F.f = 91
    puts G.new.wrapper
    puts H.singleton_class::H2.h # error: Dynamic constant references are unsupported `<emptyTree>::<constant:H>.singleton_class()::<constant:H2>`
end
main
