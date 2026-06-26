# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-method: true
# assert-no-code-action: refactor.extract

class UnsupportedSelections
  extend T::Sig

  sig {params(flag: T::Boolean).void}
  def if_branch_extract(flag)
    result = T.let(0, Integer)
    if flag
      x = 5; result = x * 2
#     ^^^^^^^^^^^^^^^^^^^^^^ apply-code-action: [A] Extract Method
    end
    puts result
  end

  sig {params(flag: T::Boolean).void}
  def else_branch_extract(flag)
    result = T.let(0, Integer)
    if flag
      result = 1
    else
      x = 10; result = x + 5
#     ^^^^^^^^^^^^^^^^^^^^^^^ apply-code-action: [B] Extract Method
    end
    puts result
  end

  sig {params(flag: T::Boolean).void}
  def extract_inline_if(flag)
    x = T.let(0, Integer)
    if flag; x = 1 else x = 2 end
#   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ apply-code-action: [C] Extract Method
    puts x
  end
end
