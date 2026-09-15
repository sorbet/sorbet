# typed: true

# This scope contains no constant references, but it still cannot claim another package's namespace.
module Second
     # ^^^^^^ error: File belongs to package `First` but defines a constant that does not match this namespace
end
