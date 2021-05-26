# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

class NotHash
  def [](arg0)
    "got key: #{arg0}"
  end
end

sig {params(x: T.any(T::Hash[T.untyped, T.untyped], NotHash)).returns(T.untyped)}
def do_aref(x)
  x[:foo]
end

hash = {foo: 627}
p hash

3.times do
  y = do_aref(hash)
  p y

  z = do_aref(NotHash.new)
  p z
end
