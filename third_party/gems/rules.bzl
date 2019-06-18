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


def _parse_package(line):
    """
    Parse an exact package specification from a single line of a Gemfile.lock.
    If the line did not contain an exact specification (something of the form
    `package (version)`, return None.
    """

    prefix = line[0:4]
    if not prefix.isspace():
        return None

    suffix = line[4:]
    if suffix[0].isspace():
        return None

    version_start = suffix.find(' (')
    if version_start < 0:
        return None

    package = suffix[0:version_start]
    version = suffix[version_start+2:-1]

    return { "name": package, "version": version }


def _parse_gemfile_lock(content):
    """
    Find lines in the content of a Gemfile.lock that look like package
    constraints.
    """

    packages = []

    for line in content.splitlines():
        info = _parse_package(line)
        if info != None:
            packages.append(info)

    return packages


def _fetch_gem(repo_ctx, cache_dir, package_name, sha256):
    """
    Download a specific version of a gem from rubygems.org and place it in
    `cache_dir`. Returns the sha256 value of the downloaded archive.
    """

    output = "{}/{}.gem".format(cache_dir, package_name)

    url = "https://rubygems.org/downloads/{}.gem".format(package_name)

    # default the sha to "" when it's unknown
    if sha256 == None:
        sha256 = ""

    result = repo_ctx.download(output = output, url = url, sha256 = sha256)

    return result.sha256


def _generate_vendor_cache(repo_ctx, gemfile_lock, deps):
    """
    Generates a BUILD file in the same location as the Gemfile.lock file in the
    original repository, that will produce the vendor/cache directory populated
    with the required gems. Returns the path to the generated vendor/cache
    directory.
    """

    package = _label_to_path(Label(gemfile_lock))

    # the name of all the gems in the vendor/cache tree
    gemfiles = ", ".join([ "\"{}.gem\"".format(_format_package(dep)) for dep in deps ])

    cache_dir = "{}/vendor/cache".format(package)

    repo_ctx.template(
        "{}/BUILD".format(cache_dir),
        Label("//third_party/gems:vendor_cache.BUILD"),
        substitutions = {
            "%{gemfiles}": gemfiles,
        },
        executable = False,
    )

    return cache_dir


BUNDLER_VERSIONS = {
    "2.0.1": {
        "sha256": "",
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

    # hard coded as 2.4.0 for now
    site_ruby = "lib/ruby/site_ruby/2.4.0"
    site_bin = "bin"

    substitutions = {
        "%{workspace}": repo_ctx.name,
        "%{bundler}": bundler,
        "%{site_ruby}": site_ruby,
        "%{site_bin}": site_bin,
    }

    repo_ctx.template(
        "setup_bundler.sh",
        Label("//third_party/gems:setup_bundler.sh"),
        executable = True,
        substitutions = substitutions,
    )

    repo_ctx.execute([ "./setup_bundler.sh" ])

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

    repo_ctx.template(
        "bundler/bundle-env.sh",
        Label("//third_party/gems:bundle-env.sh"),
        executable = True,
        substitutions = substitutions,
    )


def _impl(repo_ctx):

    known_shas = {}
    fetched = False
    gemfile_deps = {}

    _setup_bundler(repo_ctx)

    # parse all gemfiles, and unique dependencies
    for gemfile_lock in repo_ctx.attr.gemfile_locks:
        deps = _parse_gemfile_lock(_read_file(repo_ctx, Label(gemfile_lock)))

        cache_dir = _generate_vendor_cache(repo_ctx, gemfile_lock, deps)

        # fetch all the deps, sharing known sha256 values between downloads
        for dep in deps:
            package_name = _format_package(dep)

            # fall back on environmentally provided gem shas
            sha256 = known_shas.get(package_name, repo_ctx.attr.gems.get(package_name))
            if sha256 == None:
                fetched = True

            sha256 = _fetch_gem(repo_ctx, cache_dir, package_name, sha256)
            known_shas[package_name] = sha256

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
        )
    },
)
