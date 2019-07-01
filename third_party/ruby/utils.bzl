def install_file(name, src, out):
    """
    Install a single file, using `install`. Returns out, so that it can be easily
    embedded into a filegroup rule.
    """

    native.genrule(
        name = name,
        srcs = [src],
        outs = [out],
        cmd = "install -c -m 644 $(location {}) $(location {})".format(src, out),
    )

    return out

def _validate_prefix(prefix, avoid):
    """
    Validate an install prefix.
    """

    if prefix.startswith(avoid):
        rest = prefix[0:len(avoid)]
        return rest != "/" and rest != ""

    return True

def install_dir(src_prefix, out_prefix):
    """
    Copy all files under `src_prefix` to `out_prefix`. Returns the list of out
    files, for use in a `filegroup` rule.
    """

    if not _validate_prefix(src_prefix, out_prefix):
        fail(msg = "invalid src_prefix")

    if not _validate_prefix(out_prefix, src_prefix):
        fail(msg = "invalid out_prefix")

    # normalize src_prefix to end with a trailing slash
    prefix_len = len(src_prefix)
    if src_prefix != "" and src_prefix[-1] != "/":
        prefix_len += 1

    # normalize out_prefix to end with a trailing slash
    if out_prefix != "" and out_prefix[-1] != "/":
        out_prefix += "/"

    outs = []
    for src in native.glob(["{}/**/*".format(src_prefix)]):
        out_name = "install_{}".format(src)

        base = src[prefix_len:]

        out = out_prefix + base
        outs.append(out)

        install_file(name = out_name, src = src, out = out)

    return outs
