# typed: true

module Interface
  extend T::Sig
  extend T::Helpers

  interface!

  sig.abstract { void }
  def foo; end

  sig { void }
  def bar; end
end

class Override
  extend T::Sig

  include Interface

  sig.override { void }
  def foo; end

  sig.overridable { void }
  def bar; end
end

class Incompatible
  extend T::Sig

  include Interface

  sig.override(allow_incompatible: true) { returns(Integer) }
  def foo
    123
  end
end

class Final
  extend T::Sig

  sig.final { void }
  def foo; end
end

class InvalidInvocationsOnSig
  extend T::Sig

  sig.params(a: Integer)
    # ^^^^^^ error: Cannot use `params` outside of a sig block
    # ^^^^^^ error: Method `params` does not exist on `NilClass`
  def foo(a); end

  sig.returns(Integer)
    # ^^^^^^^ error: Cannot use `returns` outside of a sig block
    # ^^^^^^^ error: Method `returns` does not exist on `NilClass`
  def bar; end

  sig.void
    # ^^^^ error: Cannot use `void` outside of a sig block
    # ^^^^ error: Method `void` does not exist on `NilClass`
  def baz; end

  sig.checked(:never)
    # ^^^^^^^ error: Cannot use `checked` outside of a sig block
    # ^^^^^^^ error: Method `checked` does not exist on `NilClass`
  def blah; end

  sig.on_failure(:soft, notify: "me")
    # ^^^^^^^^^^ error: Cannot use `on_failure` outside of a sig block
    # ^^^^^^^^^^ error: Method `on_failure` does not exist on `NilClass`
  def bip; end

  sig.type_parameters(:T)
    # ^^^^^^^^^^^^^^^ error: Cannot use `type_parameters` outside of a sig block
    # ^^^^^^^^^^^^^^^ error: Method `type_parameters` does not exist on `NilClass`
  def bop; end
end

class DuplicateBlock
  extend T::Sig
  include Interface

  sig.override { void }.final { void }
                      # ^^^^^ error: Cannot add more signature statements after the declaration block
  def foo; end
end

class WithoutRuntime
  extend T::Sig
  include Interface

  T::Sig::WithoutRuntime.sig.final { void }
  def baz; end

  T::Sig::WithoutRuntime.sig.params(a: Integer)
                           # ^^^^^^ error: Cannot use `params` outside of a sig block
                           # ^^^^^^ error: Method `params` does not exist on `NilClass`
  def bar(a); end

  T::Sig::WithoutRuntime.sig.override { void }.final { void }
                                             # ^^^^^ error: Cannot add more signature statements after the declaration block
  def foo; end
end

class MissingBlocks
  extend T::Sig

  sig
# ^^^ error: Signature declarations expect a block
  def foo; end

  sig.final
    # ^^^^^ error: Signature declarations expect a block
    # ^^^^^ error: Method `final` does not exist on `NilClass`
  def bar; end

  sig.abstract
    # ^^^^^^^^ error: Signature declarations expect a block
    # ^^^^^^^^ error: Method `abstract` does not exist on `NilClass`
  def baz; end

  sig.override
    # ^^^^^^^^ error: Signature declarations expect a block
    # ^^^^^^^^ error: Method `override` does not exist on `NilClass`
  def qux; end

  sig.overridable
    # ^^^^^^^^^^^ error: Signature declarations expect a block
    # ^^^^^^^^^^^ error: Method `overridable` does not exist on `NilClass`
  def quux; end
end

class InstructionSequenceInSig
  extend T::Sig

  sig.final { Kernel.puts ""; void }
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Malformed signature: cannot have multiple statements inside a signature block
    # ^^^^^ error: Method `final` does not exist on `NilClass`
                            # ^^^^ error: Method `void` does not exist on `T.class_of(InstructionSequenceInSig)`
  def foo; end
end

class DuplicateInvocations
  extend T::Sig
  extend T::Helpers
  include Interface

  abstract!

  sig.overridable { overridable.void }
                  # ^^^^^^^^^^^ error: Duplicate invocation of `overridable` in signature declaration
  def bar; end

  sig.override { override.void }
               # ^^^^^^^^ error: Duplicate invocation of `override` in signature declaration
  def foo; end

  sig.abstract { abstract.void }
               # ^^^^^^^^ error: Duplicate invocation of `abstract` in signature declaration
  def baz; end
end

class DuplicateButMissingBlock
  extend T::Sig
  include Interface

  sig.final { void }.override
                   # ^^^^^^^^ error: Method `override` does not exist on `NilClass`
                   # ^^^^^^^^ error: Signature declarations expect a block
  def foo; end
# ^^^^^^^ error: Method `DuplicateButMissingBlock#foo` implements an abstract method `Interface#foo` but is not declared with `override.`
end

class InvalidInvocationAfterBlock
  extend T::Sig

  sig.final { void }.checked(:never)
                   # ^^^^^^^ error: Cannot use `checked` outside of a sig block
                   # ^^^^^^^ error: Method `checked` does not exist on `NilClass`
  def foo; end
end

module ValidInterface
  extend T::Sig
  extend T::Helpers

  interface!

  sig.abstract { void }
  def foo; end

  sig.abstract { void }
  def bar; end

  sig.abstract { void }
  def baz; end

  sig.abstract { void }
  def qux; end
end

class ValidDoubleChain
  extend T::Sig
  include ValidInterface

  sig.override.final { void }
  def foo; end

  sig(:final).override { void }
  def bar; end

  sig(:final).override(allow_incompatible: true) { void }
  def baz; end

  sig.override.final { void }
  def qux; end
end

class IncorrectOverrideOnFinal < ValidDoubleChain
  extend T::Sig

  sig.override { void }
  def foo; end
# ^^^^^^^ error: `ValidDoubleChain#foo` was declared as final and cannot be overridden by `IncorrectOverrideOnFinal#foo`

  sig.override { void }
  def bar; end
  # ^^^^^^^ error: `ValidDoubleChain#bar` was declared as final and cannot be overridden by `IncorrectOverrideOnFinal#bar

  sig.override { void }
  def baz; end
  # ^^^^^^^ error: `ValidDoubleChain#baz` was declared as final and cannot be overridden by `IncorrectOverrideOnFinal#baz

  sig.override { void }
  def qux; end
  # ^^^^^^^ error: `ValidDoubleChain#qux` was declared as final and cannot be overridden by `IncorrectOverrideOnFinal#qux
end
