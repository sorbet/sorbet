# error: File `test/testdata/packager/unpackaged-error/gem.rbi` does not belong to a package
# typed: true

# rbi files should not be exempt from packaging rules -- this will also error with an unpackaged error.

module ::Minitest

end

