# This is a templated build file that will export all downloaded gems in the
# "//gems" package. The `quoted_gems` template variable is supplied by `rules.bzl`.
exports_files([ {{quoted_gems}} ])

# This is so that depending on `@gems//gems` will bring in all gemfiles.
filegroup(
    name = "gems",
    srcs = [ {{quoted_gems}} ],
    visibility = [ "//visibility:public" ],
)
