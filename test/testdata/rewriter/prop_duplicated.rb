# typed: strict

class A < T::Struct

  const :name, String
  prop :name, Integer
# ^^^^^^^^^^^^^^^^^^^ error: The `prop :name` is defined multiple times

  prop :age, Integer
  const :name, Float
# ^^^^^^^^^^^^^^^^^^ error: The `const :name` is defined multiple times

  const :age, Float
# ^^^^^^^^^^^^^^^^^ error: The `const :age` is defined multiple times

  prop :"foo 'bar", Integer
  prop :"foo 'bar", Integer
# ^^^^^^^^^^^^^^^^^^^^^^^^^ error: The `prop :"foo \'bar"` is defined multiple times
end
