# TODO(jez) What should this file be named?

# def _try_openssl_dir(ctx, openssl_dir):
#     openssl_path = ctx.path(openssl_dir)
#     if openssl_path.exists:
#         native.new_local_repository(
#             name = ctx.name,
#             path = openssl_dir,
#             build_file = "@com_stripe_ruby_typer//third_party/openssl:darwin.BUILD",
#         )
#         return True
#     else:
#         return False

def _find_macos_openssl_dir(ctx):
    # TODO(jez) Could take this list as an attr if we want, for linux support.
    openssl_dirs = [
        "/usr/local/opt/openssl@1.1",
        "/opt/homebrew/opt/openssl@1.1",
        "/usr/local/opt/openssl",
        "/opt/homebrew/opt/openssl",
    ]

    for openssl_dir in openssl_dirs:
        if ctx.path(openssl_dir).exists:
            return openssl_dir

    return None

def _system_openssl_repository(ctx):
    ctx.file("WORKSPACE", "workspace(name = {name})\n".format(name = repr(ctx.name)))
    ctx.file("BUILD.bazel", ctx.read(ctx.attr.build_file))

    # TODO(jez) Could alternatively case on ctx.os.name here to only create one
    # repo, called like @system_openssl or something, instead of always
    # creating both @system_ssl_darwin and @system_ssl_linux regardless of OS
    # os_name = ctx.os.name
    # if os_name == "linux":
    #     node = Label("@nodejs_linux_amd64//:bin/node")
    # elif os_name == "mac os x":
    #     node = Label("@nodejs_darwin_amd64//:bin/node")
    # else:
    #     fail("Unsupported platform: {}".format(os_name))

    openssl_dir = _find_macos_openssl_dir(ctx)
    if openssl_dir == None:
        fail("TODO(jez) better error message")

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
    },
)
