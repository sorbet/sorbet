# typed: true
# selective-apply-code-action: refactor.rewrite
extend T::Sig

class A
  extend T::Sig

  def no_sig(x)
    # | apply-code-action: [A] Convert to singleton class method (best effort)
    puts "Hello, peter."
  end
end
