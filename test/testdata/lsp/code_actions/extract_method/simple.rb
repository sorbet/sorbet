# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-method: true

class SimpleExtract
  extend T::Sig

  sig {void}
  def single_statement
    puts "hello"
#   ^^^^^^^^^^^^ apply-code-action: [A] Extract Method
  end
end
