# typed: true
# selective-apply-code-action: refactor
# enable-experimental-lsp-extract-to-variable: true

class BlockArg
  extend T::Sig

  sig { void }
  def caller
    with_block(1) do |x|
#   ^^^^^^^^^^ error: Method `with_block` does not exist on `BlockArg`
#     ^ apply-code-action: [A] Create missing method
      puts x
    end
  end
end
