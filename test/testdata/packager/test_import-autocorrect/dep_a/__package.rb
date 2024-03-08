# frozen_string_literal: true
# typed: strict
# enable-packager: true
# selective-apply-code-action: quickfix

 class DepA < PackageSpec
#^^^^^^^^^^^^^^^^^^^^^^^^ error: Package `DepA` is missing imports
     # ^^^^ apply-code-action: [2] Fix package issues
  test_import DepB
  export DepA::ExportedItem
end
