# typed: true
# selective-apply-code-action: refactor

class SingleArg
  extend T::Sig

  sig { void }
  def caller
    greet(false)
#   ^^^^^ error: Method `greet` does not exist on `SingleArg`
#     ^ apply-code-action: [A] Create missing method
  end
end
