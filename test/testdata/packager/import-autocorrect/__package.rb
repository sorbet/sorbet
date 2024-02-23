# frozen_string_literal: true
# typed: strict
# enable-packager: true
# selective-apply-code-action: quickfix

  class Package < PackageSpec
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Package `Package` is missing imports
    # ^^^^^ apply-code-action: [1] Fix package issues
end
