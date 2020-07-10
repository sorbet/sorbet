# typed: strict

class A
  extend T::Sig

  sig {void}
  def self.get_exported_item
  end
end
