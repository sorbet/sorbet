# error: File `test/testdata/packager/unpackaged-error/unpackaged-error.rb` does not belong to a package
# typed: true
# enable-packager: true

# Doesn't matter what the contents are. This file is unpackaged and will have an error.
# This is not in test/testdata since we that test runner can't make an assertion on the first line of a file, which is
# where this error appears.
