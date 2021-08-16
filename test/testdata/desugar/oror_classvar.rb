# typed: true

extend T::Sig

class Config
  @@supported_methods ||= T.let(nil, T.nilable(T::Array[String]))

  extend T::Sig

  sig {returns(T::Array[String])}
  def self.supported_methods
    @@supported_methods ||= begin
                              temp = T.let(['fast', 'slow', 'special'].uniq.freeze, T.nilable(T::Array[String]))
                              temp
                            end
    T.reveal_type(@@supported_methods) # error: Revealed type: `T.nilable(T::Array[String])`
    T.must(@@supported_methods)
  end
end
