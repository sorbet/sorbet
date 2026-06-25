# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-method: true

class LivenessLoopIterations
  extend T::Sig

  sig {void}
  def accumulator_in_while
    sum = T.let(0, Integer)
    i = T.let(0, Integer)
    while i < 10
      sum = sum + i; i = i + 1
#     ^^^^^^^^^^^^^^^^^^^^^^^^ apply-code-action: [A] Extract Method
    end
    puts sum
  end

  sig {void}
  def multiple_loop_vars
    a = T.let(0, Integer)
    b = T.let(1, Integer)
    while a < 100
      temp = a; a = b; b = temp + b
#     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ apply-code-action: [B] Extract Method
    end
    puts a
  end
end
