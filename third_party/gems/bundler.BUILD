
filegroup(
    name = "runtime_deps",
    srcs = [
        "{{site_bin}}/bundle",
        "{{site_bin}}/bundler",
    ] + glob([ {{site_ruby_glob}} ]),
    visibility = [ "//visibility:public" ],
)

sh_binary(
    name = "bundler",
    data = [ ":runtime_deps" ],
    srcs = [ "bundle.sh" ],
    visibility = [ "//visibility:public" ],
)

sh_binary(
    name = "bundle",
    data = [ ":runtime_deps" ],
    srcs = [ "bundle.sh" ],
    visibility = [ "//visibility:public" ],
)
