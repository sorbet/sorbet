# typed: strict

module Config
  extend T::Sig

  sig {returns(T::Array[String])}
  def self.supported_methods
    @supported_methods ||= T.let(['fast', 'slow', 'special'].uniq.freeze, T.nilable(T::Array[String]))
    T.reveal_type(@supported_methods) # error: Revealed type: `T.nilable(T::Array[String])`
    T.must(@supported_methods)
  end
end
