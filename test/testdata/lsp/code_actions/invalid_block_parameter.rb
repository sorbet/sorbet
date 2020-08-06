# typed: true
# exhaustive-apply-code-action: true

class A
  extend T::Sig

  sig do
    params(block: String).returns(String)
                # ^^^^^^ error: Malformed `sig`: block parameters should be defined as callable objects
                # ^^^^^^ apply-code-action: [A] Wrap in T.proc
  end
  def with_block(&block)
    yield
  end
end
