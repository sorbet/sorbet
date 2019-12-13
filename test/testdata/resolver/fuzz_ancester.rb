# typed: strict
class A < b # error: Superclasses must only contain constant literals
end
class A < b::C # error: Superclasses must only contain constant literals
end
class A
  include b
#         ^ error: `include` must only contain constant literals
# ^^^^^^^^^ error: Expected `Module` but found `NilClass` for argument `arg0`
end
class A
  extend b
#        ^ error: `extend` must only contain constant literals
# ^^^^^^^^ error: Expected `Module` but found `NilClass` for argument `arg0`
end
module B; end
class A
  include B, c
#            ^ error: `include` must only contain constant literals
# ^^^^^^^^^^^^ error: Expected `Module` but found `NilClass` for argument `arg0`
end
