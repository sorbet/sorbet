# typed: true

class A
  module B
    module C; end
  end

  include self::B
  #       ^^^^^^^ error: `include` must only contain constant literals
  #       ^^^^^^^ error: Dynamic constant references are unsupported
  extend self::B
  #      ^^^^^^^ error: `extend` must only contain constant literals
  #      ^^^^^^^ error: Dynamic constant references are unsupported
  include self::B::C
  #       ^^^^^^^^^^ error: `include` must only contain constant literals
  #       ^^^^^^^ error: Dynamic constant references are unsupported

  include (1+1)::B
  #       ^^^^^^^^ error: `include` must only contain constant literals
  #       ^^^^^^^^ error: Dynamic constant references are unsupported
  include foo::B
  #       ^^^^^^ error: `include` must only contain constant literals
  #       ^^^^^^ error: Dynamic constant references are unsupported
  #       ^^^ error: Method `foo` does not exist on `T.class_of(A)`
end

class C1 < self::B; end
#          ^^^^^^^ error: Superclasses must only contain constant literals
class C2 < (1+1)::B; end
#          ^^^^^^^^ error: Superclasses must only contain constant literals
class C3 < foo::B; end
#          ^^^^^^ error: Superclasses must only contain constant literals
