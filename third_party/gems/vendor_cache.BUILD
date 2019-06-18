
filegroup(
    name = "cache",
    srcs = [ %{gemfiles} ],
    visibility = [ "//visibility:public" ],
)

# Generate a stable file that can be used with `location`.
genrule(
    name = "generate-token",
    outs = [ "token" ],
    cmd = "touch $@",
    visibility = [ "//visibility:public" ],
)
