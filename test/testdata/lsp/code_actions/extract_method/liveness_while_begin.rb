# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-method: true

class LivenessWhileBegin
  extend T::Sig

  sig {void}
  def side_effects_in_body
    x = T.let(0, Integer)
    while (x = x + 1; x < 10)
      puts x; puts x * 2
#     ^^^^^^^^^^^^^^^^^^ apply-code-action: [A] Extract Method
    end
  end
end
