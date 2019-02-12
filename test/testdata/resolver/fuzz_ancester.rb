# typed: strict
class A < b # error: Superclasses must only contain simple expressions
end
class A < b::C # error: Superclasses must only contain simple expressions
end
class A
  include b # error: `include` must only contain simple expressions
end
class A
  extend b # error: `extend` must only contain simple expressions
end
module B; end
class A
  include B, c
#            ^ error: `include` must only contain simple expressions
end
