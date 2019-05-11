# typed: strict
extend T::Sig
sig do
  params(
      arg: BasicObject,
  )
  .void
end
def undef(*arg); end

def foo
end
undef foo # error: Unsuppored method: undef
