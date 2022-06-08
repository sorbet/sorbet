# compiled: true
# typed: true

class ForgotTStructOrTProps
  prop :foo, Integer
# ^^^^               error: `prop` does not exist
# ^^^^^^^^^^^^^^^^^^ error: `decorator` does not exist
  const :bar, Integer
# ^^^^^               error: `const` does not exist
# ^^^^^^^^^^^^^^^^^^^ error: `decorator` does not exist
end
