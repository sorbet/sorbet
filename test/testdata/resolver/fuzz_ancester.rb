# typed: strict
class A < b # error: Superclasses must only contain constant literals
end
class A < b::C # error: Superclasses must only contain constant literals
end
class A
  include b
  #       ^ error: `include` must only contain constant literals
  #       ^ error: Method `b` does not exist on `T.class_of(A)`
end
class A
  extend b
  #      ^ error: `extend` must only contain constant literals
  #      ^ error: Method `b` does not exist on `T.class_of(A)`
end
module B; end
class A
  include B, c
  #          ^ error: `include` must only contain constant literals
  #          ^ error: Method `c` does not exist on `T.class_of(A)`
end
