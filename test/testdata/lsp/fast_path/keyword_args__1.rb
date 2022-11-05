# typed: true

module A
  extend T::Sig

  sig {params(a: String).void}
  def self.some_method(a)
  end
end
