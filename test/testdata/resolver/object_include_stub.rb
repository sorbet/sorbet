# typed: true
class Object
  include(E) # error: Unable to resolve constant `E`
        # ^ error: Circular dependency
end
