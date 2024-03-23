# frozen_string_literal: true
# typed: strict

class DepA::ExportedItem
  extend T::Sig

  sig {void}
  def self.get_exported_item
    DepB::ExportedItem.new
  # ^^^^^^^^^^^^^^^^^^ error: Used `test_import` constant `DepB::ExportedItem` in non-test file
    DepB::ExportedItem.new
  # ^^^^^^^^^^^^^^^^^^ error: Used `test_import` constant `DepB::ExportedItem` in non-test file
    DepB::ExportedItem.new
  # ^^^^^^^^^^^^^^^^^^ error: Used `test_import` constant `DepB::ExportedItem` in non-test file
  end
end
