# typed: strict

class A < T::Struct

  const :name, String
  prop :name, Integer
# ^^^^^^^^^^^^^^^^^^^ error: prop is defined multiple times

  prop :age, Integer
  const :name, Float
# ^^^^^^^^^^^^^^^^^^ error: const is defined multiple times

  const :age, Float
# ^^^^^^^^^^^^^^^^^ error: const is defined multiple times

end
