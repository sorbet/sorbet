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

BUNDLER_VERSIONS = {
    "2.0.1": {
        "sha256": "c7e38039993c9c2edc27397aef4a3370a4b35c7fae3d93e434e501c4bd7656ea",
    },
}

def _setup_bundler(repo_ctx):
    """
    Download and configure the specified version of bundler.
    """

    bundler_version = repo_ctx.attr.bundler_version

    bundler_info = BUNDLER_VERSIONS.get(bundler_version)
    if bundler_info == None:
        fail(msg = "Unknown bundler version: {}".format(bundler_version))

    sha256 = bundler_info.get("sha256", "")

    bundler = "bundler"

    repo_ctx.download_and_extract(
        output = "{}/extracted".format(bundler),
        url = "https://rubygems.org/downloads/bundler-{}.gem".format(bundler_version),
        sha256 = sha256,
        type = "tar",
    )

    site_bin = "bin"

    site_ruby_glob = ", ".join([
        "\"lib/ruby/site_ruby/{}/**/*.rb\"".format(version)
        for version in RUBY_VERSIONS
    ])

    substitutions = {
        "{{workspace}}": repo_ctx.name,
        "{{bundler}}": bundler,
        "{{site_bin}}": site_bin,
        "{{site_ruby_glob}}": site_ruby_glob,
    }

    repo_ctx.template(
        "setup_bundler.sh",
        Label("//third_party/gems:setup_bundler.sh"),
        executable = True,
        substitutions = substitutions,
    )

    repo_ctx.execute(["./setup_bundler.sh"] + RUBY_VERSIONS)

    repo_ctx.template(
        "bundler/BUILD",
        Label("//third_party/gems:bundler.BUILD"),
        executable = False,
        substitutions = substitutions,
    )

    repo_ctx.template(
        "bundler/bundle.sh",
        Label("//third_party/gems:bundle.sh"),
        executable = True,
        substitutions = substitutions,
    )

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

def _setup_tests(repo_ctx):
    """
    Download requirements for testing the bundler implementation.
    """

    # one gem to support the testing
    repo_ctx.download(
        output = "test/vendor/cache/cantor-1.2.1.gem",
        url = "https://rubygems.org/downloads/cantor-1.2.1.gem",
        sha256 = "f9c2c3d2ff23f07908990a891d4d4d53e6ad157f3fe8194ce06332fa4037d8bb",
    )

    repo_ctx.template(
        "test/smoke_test.bzl",
        Label("//third_party/gems:test/smoke_test.bzl"),
        executable = False,
    )

    repo_ctx.template(
        "test/BUILD",
        Label("//third_party/gems:test/test.BUILD"),
        executable = False,
    )

    repo_ctx.template(
        "test/smoke_test.sh",
        Label("//third_party/gems:test/smoke_test.sh"),
        executable = True,
        substitutions = {
            "{{workspace}}": repo_ctx.name,
        },
    )

    repo_ctx.template(
        "test/Gemfile",
        Label("//third_party/gems:test/Gemfile"),
        executable = False,
    )

    repo_ctx.template(
        "test/Gemfile.lock",
        Label("//third_party/gems:test/Gemfile.lock"),
        executable = False,
    )

def _impl(repo_ctx):
    fetched = False
    gemfile_deps = {}

    _setup_build_defs(repo_ctx)

    _setup_tests(repo_ctx)

    _setup_bundler(repo_ctx)

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
            "bundler_version": repo_ctx.attr.bundler_version,
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
        "bundler_version": attr.string(
            default = "2.0.1",
            doc = "The version of bundler to install",
        ),
        "debug": attr.bool(
            default = False,
            doc = "Emit debug prints",
        ),
    },
)
