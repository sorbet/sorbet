# frozen_string_literal: true
# typed: strict

class Package::A
  extend T::Sig

  sig {returns(Dep::ExportedItem)} # error: No import provides `Dep::ExportedItem`
  def self.get_exported_item
    Dep::ExportedItem.new # error: No import provides `Dep::ExportedItem`
  end
end
