# typed: strict

class ConstantAnnotatedInRbi
  MY_HASH = T.let(T.unsafe(nil), T::Hash[String, String])
  UNCHECKED_HASH = T.let(T.unsafe(nil), T::Hash[String, String], checked: false)
end
