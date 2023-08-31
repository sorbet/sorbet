# typed: true

class A < T::Struct
  extend T::Sig

  sig {void}
  def self.bar
    #      ^ def: self_bar
  end

  prop :foo, String, factory: -> {A.bar}
  #                                 ^ usage: self_bar
  #                                 ^ hover: sig { void }
  #                                 ^ hover: def self.bar; end
end
