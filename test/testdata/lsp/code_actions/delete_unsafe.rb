# typed: true
# selective-apply-code-action: refactor.rewrite

T.unsafe(0).even?
# | apply-code-action: [A] Delete T.unsafe

T.unsafe(
# | apply-code-action: [B] Delete T.unsafe
  0
).even?

_foo = T
  .unsafe(0)
  # | apply-code-action: [C] Delete T.unsafe
  .even?
