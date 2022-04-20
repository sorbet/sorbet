# frozen_string_literal: true
# typed: strict

class Package::A
  extend T::Sig

  sig {returns(Dep::ExportedItem)} # error: No import provides `Dep`
  def self.get_exported_item
    Dep::ExportedItem.new # error: No import provides `Dep`
  end
end
