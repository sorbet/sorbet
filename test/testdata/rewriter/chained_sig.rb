# typed: true

module Interface
  extend T::Sig
  extend T::Helpers

  interface!

  sig.abstract { void }
  def foo; end
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
# ^^^^^^^^^^^^^^^^^^^^^^ error: Cannot use `params` outside of a sig block
# ^^^^^^^^^^^^^^^^^^^^^^ error: Method `params` does not exist on `NilClass`
  def foo(a); end

  sig.returns(Integer)
# ^^^^^^^^^^^^^^^^^^^^ error: Cannot use `returns` outside of a sig block
# ^^^^^^^^^^^^^^^^^^^^ error: Method `returns` does not exist on `NilClass`
  def bar; end

  sig.void
# ^^^^^^^^ error: Cannot use `void` outside of a sig block
# ^^^^^^^^ error: Method `void` does not exist on `NilClass`
  def baz; end

  sig.checked(:never)
# ^^^^^^^^^^^^^^^^^^^ error: Cannot use `checked` outside of a sig block
# ^^^^^^^^^^^^^^^^^^^ error: Method `checked` does not exist on `NilClass`
  def blah; end

  sig.on_failure(:soft, notify: "me")
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Cannot use `on_failure` outside of a sig block
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Method `on_failure` does not exist on `NilClass`
  def bip; end

  sig.type_parameters(:T)
# ^^^^^^^^^^^^^^^^^^^^^^^ error: Cannot use `type_parameters` outside of a sig block
# ^^^^^^^^^^^^^^^^^^^^^^^ error: Method `type_parameters` does not exist on `NilClass`
  def bop; end
end

module Interface
  extend T::Sig
  extend T::Helpers

  interface!

  sig.abstract { void }
  def foo; end
end

class DuplicateBlock
  extend T::Sig
  include Interface

  sig.override { void }.final { void }
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Cannot chain two blocks in a single `sig`
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Method `final` does not exist on `NilClass`
                              # ^^^^ error: Method `void` does not exist on `T.class_of(DuplicateBlock)`
  def foo; end
end

class WithoutRuntime
  extend T::Sig
  include Interface

  T::Sig::WithoutRuntime.sig.final { void }
  def baz; end

  T::Sig::WithoutRuntime.sig.params(a: Integer)
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Cannot use `params` outside of a sig block
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Method `params` does not exist on `NilClass`
  def bar(a); end

  T::Sig::WithoutRuntime.sig.override { void }.final { void }
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Cannot chain two blocks in a single `sig`
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Method `final` does not exist on `NilClass`
                                                     # ^^^^ error: Method `void` does not exist on `T.class_of(WithoutRuntime)`
  def foo; end
end

class MissingBlocks
  extend T::Sig

  sig
# ^^^ error: Signature declarations expect a block
  def foo; end

  sig.final
# ^^^^^^^^^ error: Signature declarations expect a block
# ^^^^^^^^^ error: Method `final` does not exist on `NilClass`
  def bar; end
end

class InstructionSequenceInSig
  extend T::Sig

  sig.final { Kernel.puts ""; void }
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Malformed signature: cannot have multiple instructions inside a signature block
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Method `final` does not exist on `NilClass`
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
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Cannot add more signature statements after the declaration block
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Method `override` does not exist on `NilClass`
  def foo; end
# ^^^^^^^ error: Method `DuplicateButMissingBlock#foo` implements an abstract method `Interface#foo` but is not declared with `override.`
end

class ValidDoubleChain
  extend T::Sig
  include Interface

  sig.override.final { void }
  def foo; end
end
