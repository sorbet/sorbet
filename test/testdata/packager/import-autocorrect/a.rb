# frozen_string_literal: true
# typed: strict

class Package::A
  extend T::Sig

  sig {void}
  def self.get_exported_item
    Dep::ExportedItem.new
  # ^^^^^^^^^^^^^^^^^ error: `Dep::ExportedItem` resolves but its package is not imported
    Dep::ExportedItem.new
  # ^^^^^^^^^^^^^^^^^ error: `Dep::ExportedItem` resolves but its package is not imported

  end
end
