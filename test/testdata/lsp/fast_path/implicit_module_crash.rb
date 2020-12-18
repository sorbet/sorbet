# typed: true

# This is a very long comment. A long comment so that the ::DRPC::SomeSubmodule definition at the end of the file
# is at a location that is not within the range of the rbupdate file.

Bundler::CRPC # error: Unable to resolve constant

module ::DRPC::SomeSubmodule; end
