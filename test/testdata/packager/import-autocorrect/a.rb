# frozen_string_literal: true
# typed: strict
# selective-apply-code-action: source.fixAll.sorbet

class Package::A
  extend T::Sig

  sig {void}
  def self.get_exported_item
    Dep::ExportedItem.new
  # ^^^               apply-code-action: [1] Apply all Sorbet fixes for file
  # ^^^^^^^^^^^^^^^^^ error: `Dep::ExportedItem` resolves but its package is not imported
    Dep::ExportedItem.new
  # ^^^               apply-code-action: [1] Apply all Sorbet fixes for file
  # ^^^^^^^^^^^^^^^^^ error: `Dep::ExportedItem` resolves but its package is not imported

  end
end
