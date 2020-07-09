# typed: strict

class A
  extend T::Sig

  sig {returns(Dep::ExportedItem)} # error: Unable to resolve constant `ExportedItem`
  def self.get_exported_item
    Dep::ExportedItem.new # error: Unable to resolve constant `ExportedItem`
  end
end
