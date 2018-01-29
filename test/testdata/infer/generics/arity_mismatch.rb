# @typed
class Generic
  T1 = T.type
  T2 = T.type
end

def use_it
  g1 = Generic[Integer].new # error: Wrong number of type parameters
  T.assert_type!(g1, Generic[Integer, T.untyped])

  g2 = Generic[Integer, String, Object].new # error: Wrong number of type parameters
  T.assert_type!(g2, Generic[Integer, String])
end
