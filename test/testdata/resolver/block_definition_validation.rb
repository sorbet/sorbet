# typed: strict

class A
  extend T::Sig

  sig do
    params(block: String).returns(String)
         # ^^^^^ error: Malformed `sig`: block parameters should be defined as callable objects
  end
  def with_block(&block)
    yield
  end
end
