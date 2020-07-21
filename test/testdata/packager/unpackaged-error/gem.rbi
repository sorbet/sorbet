# error: File `
# ^ We can’t assert the full pathname; see https://github.com/sorbet/sorbet/pull/3310
# typed: true

# rbi files should not be exempt from packaging rules -- this will also error with an unpackaged error.

module ::Minitest

end

