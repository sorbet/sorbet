# typed: __STDLIB_INTERNAL

class Concurrent::Hash < Hash
  K = type_member(:out)
  V = type_member(:out)
  Elem = type_member(:out)
end
