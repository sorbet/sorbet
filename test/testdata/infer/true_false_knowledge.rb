# typed: true

# Once upon a time, we considered a change to make it so that it's not required
# to add `T.let(..., T::Boolean)` when changing a variable inside a loop.
#
# But that had the unfortunate consequence that Sorbet stopped being able to
# see through control flow as good as it was able to before, so we tabled the
# change. The reasoning was that the error message for changing a variable in a
# loop is very clear, has an autocorrect, and the resulting `T.let`'d code is
# very obvious. But the error message if we did it the other way would have
# non-obvious error messages, like Sorbet thinking something might be nil when
# if it had been written slightly differently it would be able to know better.
#
# For more on the tabled implementation, see:
# <https://github.com/sorbet/sorbet/pull/4368>

extend T::Sig

sig {returns(T::Boolean)}
def boolean_true; true; end
sig {returns(T::Boolean)}
def boolean_false; false; end

sig {params(banking_account: T.nilable(String)).void}
def desired_behavior(banking_account)
  should_check_balance = false

  if banking_account
    should_check_balance = true
  end

  if should_check_balance
    T.reveal_type(banking_account) # error: `String`
  end
end

sig {params(banking_account: T.nilable(String)).void}
def bad_behavior_fake_method(banking_account)
  should_check_balance = boolean_false

  if banking_account
    should_check_balance = boolean_true
  end

  if should_check_balance
    T.reveal_type(banking_account) # error: `T.nilable(String)`
  end
end

sig {params(banking_account: T.nilable(String)).void}
def work_around_bad_behavior(banking_account)
  should_check_balance = !!false

  if banking_account
    should_check_balance = !!true
  end

  if should_check_balance
    T.reveal_type(banking_account) # error: `String`
  end
end
