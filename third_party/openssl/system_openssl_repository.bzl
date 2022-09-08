def _system_openssl_repository(ctx):
    ctx.file("WORKSPACE", "workspace(name = {name})\n".format(name = repr(ctx.name)))
    ctx.file("BUILD.bazel", ctx.read(ctx.attr.build_file))

    openssl_dir = None
    for current_dir in ctx.attr.openssl_dirs:
        if ctx.path(current_dir).exists:
            openssl_dir = current_dir
            break

    if openssl_dir == None:
        searched = ", ".join(ctx.attr.openssl_dirs)
        fail("Could not find openssl dir. Searched: [{}]".format(searched))

    openssl_path = ctx.path(openssl_dir)
    for openssl_child_path in openssl_path.readdir():
        ctx.symlink(openssl_child_path, openssl_child_path.basename)

system_openssl_repository = repository_rule(
    _system_openssl_repository,
    attrs = {
        "build_file": attr.label(
            allow_single_file = True,
            mandatory = True,
        ),
        "openssl_dirs": attr.string_list(
            mandatory = True,
        ),
    },
)
