# typed: true
extend T::Sig
sig do
  params(
      arg: BasicObject,
  )
  .void
end
def undef(*arg); end

undef foo, :bar, :"foo#{1}"
