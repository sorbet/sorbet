# typed: strict

class Module
  include T::Sig
end

class A
  sig {returns(String)}
  def get_user_cached
    @foo = T.let(@foo, T.nilable(String))
    T.reveal_type(@foo) # error: Revealed type: `T.nilable(String)`
    @foo ||= ENV.fetch('USER')
    T.reveal_type(@foo) # error: Revealed type: `String`
  end
end
