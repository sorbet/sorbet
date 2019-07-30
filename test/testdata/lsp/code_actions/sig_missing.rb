# typed: strict
# exhaustive-apply-code-action: true

module Bar
  def foo
# ^^^^^^^ error: This function does not have a `sig`
# ^^^^^^^ apply-code-action: [A] This function does not have a `sig`
  end
end
