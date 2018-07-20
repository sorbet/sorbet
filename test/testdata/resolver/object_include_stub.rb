# typed: true
class Object
  include(E) # error: MULTI
             # We error both about it being a stub, and then about the circular include.
end
