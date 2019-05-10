# typed: true
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
undef foo

def bar
end
undef :bar

undef :bad_method # This should error but we don't do that yet
