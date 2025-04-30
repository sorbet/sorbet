# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-method: true

class A
  def y
    1
  end
 
  def foo(x)
    a = x + y; b = x * y; c = x + y + a + b; return c
  #            ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ apply-code-action: [A] Extract Method
  end
end
