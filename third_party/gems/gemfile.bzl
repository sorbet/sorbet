def _parse_package(line):
    """
    Parse an exact package specification from a single line of a Gemfile.lock.
    The Gemfile.lock format uses two spaces for each level of indentation. The
    lines that we're interested in are nested underneath the `GEM.specs`
    section (the concrete gems required and their exact versions).

    The lines that are parsed out of the 'GEM.specs' section will have four
    leading spaces, and consist of the package name and exact version needed
    in parenthesis.

    >    gem-name (gem-version)

    What's returned is a dict that will have two fields:

    > { "name": "gem-name", "version": "gem-version" }

    in this case.

    If the line does not match that format, `None` is returned.
    """

    prefix = line[0:4]
    if not prefix.isspace():
        return None

    suffix = line[4:]
    if suffix[0].isspace():
        return None

    version_start = suffix.find(" (")
    if version_start < 0:
        return None

    package = suffix[0:version_start]
    version = suffix[version_start + 2:-1]

    return {"name": package, "version": version}

def parse_gemfile_lock(content):
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
