load("//third_party/gems:gemfile.bzl", _parse_gemfile_lock = "parse_gemfile_lock")
load("//third_party/gems:known_gems.bzl", "get_known_gem_sha256")

# hard-coding ruby versions to support
RUBY_VERSIONS = ["2.4.0", "2.6.0"]

def _label_to_path(label):
    """
    Given a Label, turn it into a path by keeping the directory part of the
    name, and attaching that to the package path.
    """

    base = label.package

    name = label.name

    last_slash = name.rfind("/")
    if last_slash >= 0:
        base = "{}/{}".format(base, name[0:last_slash])

    return base

def _read_file(repo_ctx, label):
    """
    Bazel-0.21.0 lacks `repo_ctx.read`, so this simulates it by briefly
    symlinking the file into the current repo, cat-ing it, and then removing
    the symlink.
    """

    if repo_ctx.attr.debug:
        print("Reading {}".format(label))

    repo_ctx.symlink(label, "tmp")

    result = repo_ctx.execute(["cat", "tmp"])

    if result.return_code != 0:
        fail("Failed to read file `{}`".format(label))

    repo_ctx.execute(["rm", "tmp"])

    return result.stdout

def _format_package(info):
    return "{}-{}".format(info["name"], info["version"])

def _fetch_gem(repo_ctx, package_name, sha256):
    """
    Download a specific version of a gem from rubygems.org and place it in
    `//gems`. Returns the sha256 value of the downloaded archive.
    """

    output = "gems/{}.gem".format(package_name)

    url = "https://rubygems.org/downloads/{}.gem".format(package_name)

    # default the sha to "" when it's unknown
    if sha256 == None:
        sha256 = ""

    result = repo_ctx.download(output = output, url = url, sha256 = sha256)

    return result.sha256

def _setup_build_defs(repo_ctx):
    """
    Install build_defs provided by this package.
    """

    # BUILD file that re-exports gemfile.bzl
    repo_ctx.template(
        "tools/build_defs/BUILD",
        Label("//third_party/gems:build_defs.BUILD"),
        executable = False,
    )

    # gemfile parsing and vendor/cache setup
    repo_ctx.template(
        "tools/build_defs/gemfile.bzl",
        Label("//third_party/gems:gemfile.bzl"),
        executable = False,
    )

def _setup_gem_export(repo_ctx, fetched_gems):
    """
    Install a BUILD file in //gems to re-export all fetched gems.
    """

    # build up a string that contains all the quoted gem package file names, for
    # use with the `exports_files` rule in the `gems.BUILD` template.
    quoted_gems = ", ".join(["\"{}.gem\"".format(package_name) for package_name in fetched_gems])

    repo_ctx.template(
        "gems/BUILD",
        Label("//third_party/gems:gems.BUILD"),
        substitutions = {
            "{{quoted_gems}}": quoted_gems,
        },
        executable = False,
    )

def _impl(repo_ctx):
    fetched = False
    gemfile_deps = {}

    _setup_build_defs(repo_ctx)

    gems_to_fetch = {}

    for gem, sha256 in repo_ctx.attr.gems:
        gems_to_fetch[gem] = sha256

    # collect all requested gems
    for gemfile_lock in repo_ctx.attr.gemfile_locks:
        deps = _parse_gemfile_lock(_read_file(repo_ctx, Label(gemfile_lock)))

        # fetch all the deps, sharing known sha256 values between downloads
        for dep in deps:
            package_name = _format_package(dep)

            if gems_to_fetch.get(package_name) == None:
                gems_to_fetch[package_name] = get_known_gem_sha256(package_name, "")
                if gems_to_fetch.get(package_name, "") == "":
                    # there is a gem present in a Gemfile.lock that wasn't mentioned
                    # in gems, so we know that the `gems` attr need to be fixed
                    fetched = True

    # fetch all the gems
    known_shas = {}
    for package_name in gems_to_fetch:
        known_shas[package_name] = _fetch_gem(repo_ctx, package_name, gems_to_fetch[package_name])

    # generate a build file that exports all the gems
    _setup_gem_export(repo_ctx, known_shas)

    # When we've fetched gems that lack sha256 value, emit attributes that would
    # make this hermetic.
    if fetched:
        return {
            "name": repo_ctx.attr.name,
            "gemfile_locks": repo_ctx.attr.gemfile_locks,
            "gems": known_shas,
        }
    else:
        return None

gemfile_lock_deps = repository_rule(
    implementation = _impl,
    local = True,
    attrs = {
        "gemfile_locks": attr.string_list(
            default = [],
            doc = "Gemfile.lock files to download the dependencies of",
        ),
        "gems": attr.string_dict(
            default = {},
            doc = "Specific gem versions and sha256 values",
        ),
        "debug": attr.bool(
            default = False,
            doc = "Emit debug prints",
        ),
    },
)
