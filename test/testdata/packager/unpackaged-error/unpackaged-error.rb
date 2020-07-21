# error: File `
# ^ We can’t assert the full pathname; see https://github.com/sorbet/sorbet/pull/3310
# typed: true
# enable-packager: true

# Doesn't matter what the contents are. This file is unpackaged and will have an error.
# This is not in test/testdata since we that test runner can't make an assertion on the first line of a file, which is
# where this error appears.
