# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-method: true

class LivenessComplexWhile
  extend T::Sig

  sig {void}
  def begin_condition_side_effect_body
    x = T.let(0, Integer)
    while (x = x + 1; x < 20)
      puts x
#     ^^^^^^ apply-code-action: [A] Extract Method
    end
  end

  sig {void}
  def body_modifies_condition_var
    x = T.let(0, Integer)
    while (x = x + 1; x < 10)
      x = x + 1
#     ^^^^^^^^^ apply-code-action: [B] Extract Method
    end
    puts x
  end

  sig {void}
  def read_write_multiple_loop_vars
    i = T.let(0, Integer)
    sum = T.let(0, Integer)
    while i < 10
      sum = sum + i; i = i + 1
#     ^^^^^^^^^^^^^^^^^^^^^^^^ apply-code-action: [C] Extract Method
    end
    puts sum
  end
end
