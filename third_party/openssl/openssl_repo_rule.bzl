# TODO(jez) What should this file be named?

def _try_openssl_dir(ctx, openssl_dir):
    openssl_path = ctx.path(openssl_dir)
    if openssl_path.exists:
        native.new_local_repository(
            name = ctx.name,
            path = openssl_dir,
            build_file = "@com_stripe_ruby_typer//third_party/openssl:darwin.BUILD",
        )
        return True
    else:
        return False

def _system_openssl_finder(ctx):
    if _try_openssl_dir(ctx, "/usr/local/opt/openssl@1.1"):
        return
    elif _try_openssl_dir(ctx, "/opt/homebrew/opt/openssl@1.1"):
        return
    elif _try_openssl_dir(ctx, "/usr/local/opt/openssl"):
        return
    elif _try_openssl_dir(ctx, "/opt/homebrew/opt/openssl"):
        return

system_openssl_finder = repository_rule(
    _system_openssl_finder,
)
