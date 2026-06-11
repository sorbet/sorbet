# typed: true
# selective-apply-code-action: refactor

class OnlyKwargs
  extend T::Sig

  sig { void }
  def caller
    configure(name: "test", count: 5, enabled: true)
#   ^^^^^^^^^ error: Method `configure` does not exist on `OnlyKwargs`
#     ^ apply-code-action: [A] Create missing method
  end
end
