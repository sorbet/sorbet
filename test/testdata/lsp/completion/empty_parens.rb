# typed: true
extend T::Sig

module M
  extend T::Sig

  sig { params(x: T.untyped, blk: T.nilable(Proc)).void }
  def aaa_only_on_m(x, &blk); end
end

# These cases are not great at the moment.
#
# We currently rely on getting a `SendResponse` back to show local variable
# completions inside argument lists, which is slightly odd because we then
# check whether the send's funLoc contains the completion position, and if it
# doesn't we set the completion prefix to "" and suggest calling methods that
# might not be in scope. We should _probably_ fix that, but at least some
# things get suggested there right now, even if it's not everything.

sig {params(m: M, a: Integer, b: Integer, blk: T.nilable(Proc)).void}
def AAA_example1(m, a, b, &blk)
  AAA_example1()
  #           ^^ error: Not enough arguments
  #            ^ completion: a, b, blk, m, AAA_example1, ...

  # No locals here because !isPrivateOk 🙃
  m.aaa_only_on_m()
  #              ^^ error: Not enough arguments
  #               ^ completion: aaa_only_on_m, ...

  # In addition to the above wonkiness, this case is even wonkier, because of
  # how we attempt to not show nonsensical completion request inside a method's
  # block.

  AAA_example1() {}
  #           ^ error: Not enough arguments
  #            ^ completion: (nothing)
  #               ^ completion: (nothing)

  m.aaa_only_on_m() {}
  #              ^ error: Not enough arguments
  #               ^ completion: (nothing)
  #                  ^ completion: (nothing)
end
