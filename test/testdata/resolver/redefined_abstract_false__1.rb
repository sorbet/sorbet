# typed: false

module IFoo
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.void}
  def foo; end

  # This redefinition is an error, but silenced at `# typed: false`
  sig {abstract.params(x: Integer).void}
  def foo(x); end
end
