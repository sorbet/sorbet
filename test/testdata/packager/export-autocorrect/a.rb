# frozen_string_literal: true
# typed: strict
# selective-apply-code-action: source.fixAll.sorbet

class Package::A
  extend T::Sig

  sig {void}
  def self.get_exported_item
    Dep::ExportedItem.new
  # ^^^               apply-code-action: [1] Apply all Sorbet fixes for file
  # ^^^^^^^^^^^^^^^^^ error: `Dep::ExportedItem` resolves but is not exported from `Dep`

    Dep::ExportedItem.new
  # ^^^               apply-code-action: [1] Apply all Sorbet fixes for file
  # ^^^^^^^^^^^^^^^^^ error: `Dep::ExportedItem` resolves but is not exported from `Dep`

  end
end
