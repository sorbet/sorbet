# typed: true
class Object
  include(E) # error-with-dupes: Unable to resolve constant `E`
end
