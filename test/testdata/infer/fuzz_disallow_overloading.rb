# typed: true
extend T::Sig
f
sig {void} # error: Unused type annotation. No method def before next annotation
sig {void}
def f; end
