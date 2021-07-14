# typed: true

extend T::Sig

module IFoo
  extend T::Helpers
  sealed!

  class A < T::Struct
    include IFoo
  end
end

x = T.let(IFoo::A.new, IFoo)
# Sorbet has enough information to know this is always `TrueClass`
T.reveal_type(IFoo::A === x) # error: Revealed type: `TrueClass`

sig {params(foo: IFoo).returns(Integer)}
def foo!(foo)
  # This case is exhaustive
  res = case foo
  when IFoo::A then 0
  end

  res
end
