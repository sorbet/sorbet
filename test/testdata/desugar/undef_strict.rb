# typed: strict
extend T::Sig
sig do
  params(
      arg: BasicObject,
  )
  .void
end
def undef(*arg); end

sig {void}
def foo
end
undef foo # error-with-dupes: Unsuppored method: undef
