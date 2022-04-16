# frozen_string_literal: true
# typed: strict

class Package::A
  extend T::Sig

  sig {returns(Dep::ExportedItem)} # error: Unable to resolve constant `Dep`
  def self.get_exported_item
    Dep::ExportedItem.new # error: Unable to resolve constant `Dep`
  end
end
