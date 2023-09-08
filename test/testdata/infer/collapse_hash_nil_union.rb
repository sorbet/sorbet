# typed: true
extend T::Sig

sig {returns(T.nilable(T::Hash[String, String]))}
def returns_nilable_hash
  {}
end

if T.unsafe(nil)
  foo =
    if T.unsafe(nil)
      unless T.unsafe(nil)
        returns_nilable_hash
      else
        nil
      end
    else
      T::Hash[String, String].new
    end
  T.reveal_type(foo) # error: T.nilable(T::Hash[String, String])
end
