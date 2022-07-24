# typed: strict

module Config
  extend T::Sig

  sig {returns(T::Array[String])}
  def self.supported_methods
    @supported_methods ||= T.let(['fast', 'slow', 'special'].uniq.freeze, T.nilable(T::Array[String]))
    T.reveal_type(@supported_methods) # error: Revealed type: `T::Array[String]`
    T.must(@supported_methods) # error: `T.must` called on `T::Array[String]`, which is never `nil`
  end

  sig {returns(String)}
  def self.initialized_to_nilable
    @initialized_to_nilable ||= T.let(nil, T.nilable(String))
  end # error: Expected `String` but found `T.nilable(String)` for method result type

  sig {returns(T::Boolean)}
  def self.lazy_boolean
    @lazy_boolean ||= T.let(false, T.nilable(T::Boolean))
  end
end
