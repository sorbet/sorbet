# frozen_string_literal: true
# typed: strict

class Package::A
  extend T::Sig

  sig {returns(Dep::ExportedItem)} # error: `Dep::ExportedItem` resolves but its package is not imported
  def self.get_exported_item
    Dep::ExportedItem.new # error: `Dep::ExportedItem` resolves but its package is not imported
  end
end
