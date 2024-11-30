# frozen_string_literal: true
# typed: strict

class Test::Package::B
  extend T::Sig

  sig {void}
  def self.get_exported_item
    DepB::ExportedItem.new
  # ^^^^^^^^^^^^^^^^^^ error: `DepB::ExportedItem` resolves but its package is not imported
    DepB::ExportedItem.new
  # ^^^^^^^^^^^^^^^^^^ error: `DepB::ExportedItem` resolves but its package is not imported

  end
end
