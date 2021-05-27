# frozen_string_literal: true
# typed: strict

class Package::A
  extend T::Sig

  sig {void}
  def self.get_exported_item
  end
end
