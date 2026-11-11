# error: File `
# frozen_string_literal: true
# typed: true
# enable-packager: true
# enable-package-directed: true

# stratum: 0

# This file has no package, but this constant isn't in any package's
# namespace, so packaged code can use it.
module UnpackagedCode
end
