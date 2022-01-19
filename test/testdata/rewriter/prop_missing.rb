# typed: true

class ForgotTStructOrTProps
  prop :foo, Integer
# ^^^^               error: `prop` does not exist
  const :bar, Integer
# ^^^^^               error: `const` does not exist
end
