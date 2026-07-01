# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-method: true

class LivenessConditional
  extend T::Sig

  sig {params(flag: T::Boolean).void}
  def if_branch_assigns(flag)
    x = T.let(0, Integer)
    if flag; x = 1 end
#   ^^^^^^^^^^^^^^^^^^ apply-code-action: [A] Extract Method
    puts x
  end
end
