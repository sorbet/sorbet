# typed: strict

# NOTE: this package exists to work around an issue where old-style test namespaces would cause
# the `Consumer` package to be processed twice: once for application code and once for test code.
# As the error we care about is reported once when we build the package DB, the second set of
# errors reported for the file ends up clearing those, and we see the slow path run without
# reporting packager errors.
#
# This file marks `Consumer` as migrated to test-packages, and it only runs through the pipeline
# a single time as a result.
class Consumer::Test < PackageSpec
  test!
  
  import Consumer
end
