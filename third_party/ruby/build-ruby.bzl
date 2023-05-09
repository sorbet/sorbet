load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "C_COMPILE_ACTION_NAME")
load("@bazel_tools//tools/cpp:toolchain_utils.bzl", "find_cpp_toolchain")

_HERMETIC_TAR = """
# If we have gtar installed (darwin), use that instead
if hash gtar 2>/dev/null; then
    alias tar=gtar
fi
hermetic_tar() {
    SRC="$1"
    OUT="$2"
    shift 2
    PATHS="${@:-.}"
    # Check if our tar supports --sort (we assume --sort support implies --mtime support)
    if [[ $(tar --sort 2>&1) =~ 'requires an argument' ]]; then
        tar --sort=name --owner=0 --group=0 --numeric-owner --mtime='UTC 1970-01-01 00:00' -h -C "$SRC" -rf "$OUT" $PATHS
    elif [[ $(tar --mtime 2>&1) =~ 'requires an argument' ]]; then
        (cd "$SRC" && find . -print0) | LC_ALL=C sort -z | tar -h -C "$SRC" --no-recursion --null -T - --owner=0 --group=0 --numeric-owner --mtime='UTC 1970-01-01 00:00' -rf "$OUT" $PATHS
    else
        # Oh well, no hermetic tar for you
        tar -h -C "$SRC" -rf "$OUT" $PATHS
    fi
}
"""

_RUNFILES_BASH = """
# --- begin runfiles.bash initialization v2 ---
# Copy-pasted from the Bazel Bash runfiles library v2.
set -uo pipefail; f=bazel_tools/tools/bash/runfiles/runfiles.bash
source "${RUNFILES_DIR:-/dev/null}/$f" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "${RUNFILES_MANIFEST_FILE:-/dev/null}" | cut -f2- -d' ')" 2>/dev/null || \
  source "$0.runfiles/$f" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "$0.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "$0.exe.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \
  { echo>&2 "ERROR: cannot find $f"; exit 1; }; f=; set -e
# --- end runfiles.bash initialization v2 ---
"""

_BUILD_RUBY = """#!/bin/bash

set -euo pipefail

base="$PWD"
out_dir="$base/{toolchain}"

declare -a inc_path=()
for path in {hdrs}; do
  inc_path=("${{inc_path[@]:-}}" "-isystem" "$base/$path")
done

declare -a lib_path=()
for path in {libs}; do
  lib_path=("${{lib_path[@]:-}}" "-L$base/$path")
done

build_dir="$(mktemp -d)"

# Add the compiler's directory the PATH, as this fixes builds with gcc
export PATH="$(dirname "{cc}"):$PATH"

# Copy everything to a separate directory to get rid of symlinks:
# tool/rbinstall.rb will explicitly ignore symlinks when installing files,
# and this feels more maintainable than patching it.
cp -aL "{src_dir}"/* "$build_dir"

{install_extra_srcs}
{install_append_srcs}

pushd "$build_dir" > /dev/null

run_cmd() {{
    if ! "$@" < /dev/null >> build.log 2>&1; then
        echo "command $@ failed!"
        cat build.log
        echo "build dir: $build_dir"
        exit 1
    fi
}}

run_cmd sed -i.bak -e 's@^COMMONOBJS\\( *\\)=@COMMONOBJS\\1= {extra_srcs_object_files}@' common.mk
run_cmd rm -f common.mk.bak

# This is a hack to get C level backtraces working. (The default autoconf test
# for backtraces uses a sigaltstack size that is too small, so the SIGSEGV
# signal handler itself causes a SIGSEGV). This value was plucked from signal.c:
# https://github.com/ruby/ruby/blob/v2_6_5/signal.c#L568
run_cmd sed -i.bak -e 's@SIGSTKSZ@16*1024@' configure
run_cmd rm -f configure.bak

# This is a hack. The configure script builds up a command for compiling C
# files that includes `-fvisibility=hidden`. To override it, our flag needs to
# come after, so we inject a flag right before the `-o` option that comes near
# the end of the command via OUTFLAG.
OUTFLAG="-fvisibility=default -o" \
CC="{cc}" \
CFLAGS="{copts}" \
CXXFLAGS="{copts}" \
CPPFLAGS="{sysroot_flag} ${{inc_path[*]:-}} {cppopts}" \
LDFLAGS="{sysroot_flag} ${{lib_path[*]:-}} {linkopts}" \
run_cmd ./configure \
        {configure_flags} \
        --enable-load-relative \
        --with-destdir="$out_dir" \
        --with-rubyhdrdir='${{includedir}}' \
        --with-rubyarchhdrdir='${{includedir}}/ruby-arch' \
        --disable-install-doc \
        --prefix=/

run_cmd make V=1 -j8
run_cmd make V=1 install

ruby_version=$(./miniruby -r ./rbconfig.rb -e 'puts "#{{RbConfig::CONFIG["MAJOR"]}}.#{{RbConfig::CONFIG["MINOR"]}}"')

internal_incdir="$base/{internal_incdir}"

mkdir -p "$internal_incdir"

cp *.h *.inc "$internal_incdir"

find ccan -type d | while read dir; do
  mkdir -p "$internal_incdir/$dir"
done

find ccan -type f -name \\*.h | while read file; do
  cp $file "$internal_incdir/$file"
done

# Put the installed ruby into our path to run gem commands
export PATH="$out_dir/bin:$PATH"

cp "$base/{rubygems}" rubygems-update.gem
run_cmd gem install --no-document --local --env-shebang rubygems-update.gem

# Overwrites the version of the `gem` and `bundle` commands inside the Ruby
# distribution itself with newer versions (the specific newer versions are
# pinned for a given rubygems-update version).
run_cmd update_rubygems

# Fix the shebang in the wrapper scripts updated by 'update_rubygems'
ruby_path="$(which ruby)"
for file in gem bundle rake; do
    sed -i'' -e "1s|$ruby_path|/usr/bin/env ruby|" "$out_dir/bin/$file"
done

# rubygems-update isn't needed after update_rubygems has been run
run_cmd gem uninstall rubygems-update

# NOTE: bundle and bundler are the same, but bundler doesn't get updated by
# rubygems_update.
cp "$out_dir/bin/bundle" "$out_dir/bin/bundler"

{install_gems}

popd > /dev/null

rm -rf "$build_dir"

"""

# Attempt to preserve the directory structure of the extra source files, so that things like #include
# directives "just work". For example, `#include "foo/foo.h"` would mean `foo.h` needs to be inside `foo/`
_INSTALL_EXTRA_SRC = """
mkdir -p "$build_dir/{dirname}"
cp "{file}" "$build_dir/{dirname}/{basename}"
"""

_INSTALL_APPEND_SRC = """
cat "{file}" >> "$build_dir/{target}"
"""

_INSTALL_GEM = """

# Copy {file} locally to avoid having the gem command accidentally interpret it
# as a gem name, rather than a filename.
cp "$base/{file}" package.gem

# --local to avoid going to rubygmes for the gem
# --env-shebang to not hardcode the path to ruby in the sandbox
run_cmd gem install --local --env-shebang package.gem

"""

_APPLY_PATCH = """
patch -d "$out_dir" -p1 < "{path}" >> build.log 2>&1
"""

RubyInfo = provider()

def _build_ruby_impl(ctx):
    src_dir = ctx.label.workspace_root

    # Setup toolchains
    cc_toolchain = find_cpp_toolchain(ctx)

    feature_configuration = cc_common.configure_features(
        ctx = ctx,
        cc_toolchain = cc_toolchain,
        requested_features = ctx.features,
        unsupported_features = ctx.disabled_features,
    )

    cc = cc_common.get_tool_for_action(
        feature_configuration = feature_configuration,
        action_name = C_COMPILE_ACTION_NAME,
    )

    # process deps into include and linker paths
    hdrs = {}
    libs = {}
    deps = {}

    for dep in ctx.attr.deps:
        cc_info = dep[CcInfo]
        if None == cc_info:
            fail("Dependency {} is missing a CcInfo".format(dep))

        for hdr in cc_info.compilation_context.headers.to_list():
            deps[hdr] = True

        for path in cc_info.compilation_context.system_includes.to_list():
            hdrs[path] = True

        for linker_input in cc_info.linking_context.linker_inputs.to_list():
            for lib in linker_input.libraries:
                libs[lib.dynamic_library.dirname] = True
                deps[lib.dynamic_library] = True

    hdrs = hdrs.keys()
    libs = libs.keys()
    deps = deps.keys()

    # Outputs
    binaries = [
        ctx.actions.declare_file("toolchain/bin/{}".format(binary))
        for binary in ["ruby", "erb", "gem", "irb", "rdoc", "ri", "bundle", "bundler"]
    ]

    libdir = ctx.actions.declare_directory("toolchain/lib")
    incdir = ctx.actions.declare_directory("toolchain/include")
    sharedir = ctx.actions.declare_directory("toolchain/share")

    internal_incdir = ctx.actions.declare_directory("toolchain/internal_include")

    outputs = binaries + [libdir, incdir, sharedir, internal_incdir]

    install_extra_srcs = []
    extra_srcs_object_files = []
    for extra_src in ctx.attr.extra_srcs:
        dirname = extra_src.label.package

        for file in extra_src.files.to_list():
            basename = file.basename
            install_extra_srcs.append(_INSTALL_EXTRA_SRC.format(file = file.path, dirname = dirname, basename = basename))

            if file.extension == "c":
                without_ext = basename[0:basename.rfind(".")]
                extra_obj = "{dirname}/{without_ext}.$(OBJEXT)".format(without_ext = without_ext, dirname = dirname)
                extra_srcs_object_files.append(extra_obj)

    install_append_srcs = []
    for append_src in ctx.attr.append_srcs:
        for file in append_src.files.to_list():
            install_append_srcs.append(_INSTALL_APPEND_SRC.format(file = file.path, target = file.basename))

    install_gems = [_INSTALL_GEM.format(file = file.path) for file in ctx.files.gems]

    # Build
    ctx.actions.run_shell(
        mnemonic = "BuildRuby",
        inputs = deps + ctx.files.src + ctx.files.rubygems + ctx.files.gems + ctx.files.extra_srcs + ctx.files.append_srcs,
        outputs = outputs,
        command = ctx.expand_location(_BUILD_RUBY.format(
            cc = cc,
            copts = " ".join(ctx.attr.copts),
            linkopts = " ".join(ctx.attr.linkopts),
            cppopts = " ".join(ctx.attr.cppopts),
            toolchain = libdir.dirname,
            src_dir = src_dir,
            internal_incdir = internal_incdir.path,
            hdrs = " ".join(hdrs),
            libs = " ".join(libs),
            rubygems = ctx.files.rubygems[0].path,
            configure_flags = " ".join(ctx.attr.configure_flags),
            sysroot_flag = ctx.attr.sysroot_flag,
            install_extra_srcs = "\n".join(install_extra_srcs),
            extra_srcs_object_files = " ".join(extra_srcs_object_files),
            install_append_srcs = "\n".join(install_append_srcs),
            install_gems = "\n".join(install_gems),
        )),
    )

    return [
        RubyInfo(
            toolchain = libdir.dirname,
            binaries = binaries,
            includes = incdir,
            internal_includes = internal_incdir,
            lib = libdir,
            share = sharedir,
        ),
        DefaultInfo(
            files = depset(outputs),
        ),
        CcInfo(),
    ]

_build_ruby = rule(
    attrs = {
        "src": attr.label(),
        "deps": attr.label_list(),
        "rubygems": attr.label(
            mandatory = True,
            doc = "The rubygems-update gem to install and apply",
        ),
        "gems": attr.label_list(
            doc = "Additional ruby gems to install into the ruby build",
        ),
        "extra_srcs": attr.label_list(
            doc = "A list of *.c and *.h files to treat as extra source files to libruby",
        ),
        "append_srcs": attr.label_list(
            doc = "A list of *.c files to append to the file of the same name in the ruby vm",
        ),
        "configure_flags": attr.string_list(
            doc = "Additional arguments to configure",
        ),
        "copts": attr.string_list(
            doc = "Additional copts for the ruby build",
        ),
        "cppopts": attr.string_list(
            doc = "Additional preprocessor opts for the ruby build",
        ),
        "linkopts": attr.string_list(
            doc = "Additional linkopts for the ruby build",
        ),
        "_cc_toolchain": attr.label(
            default = Label("@bazel_tools//tools/cpp:current_cc_toolchain"),
        ),
        "sysroot_flag": attr.string(),
    },
    fragments = ["cpp"],
    provides = [
        RubyInfo,
        DefaultInfo,
    ],
    toolchains = ["@bazel_tools//tools/cpp:toolchain_type"],
    implementation = _build_ruby_impl,
)

_BUILD_ARCHIVE = """#!/bin/bash

{hermetic_tar_setup}

base=$PWD
toolchain="$base/{toolchain}"

archive="$(mktemp)"

# explicitly exclude internal_incdir and static_libs
hermetic_tar "$toolchain" "${{archive}}" bin lib include share
gzip -c "${{archive}}" > "{output}"

rm "${{archive}}"
"""

def _ruby_archive_impl(ctx):
    ruby_info = ctx.attr.ruby[RubyInfo]

    archive = ctx.actions.declare_file("ruby.tar.gz")

    ctx.actions.run_shell(
        outputs = [archive],
        inputs = ruby_info.binaries + [ruby_info.includes, ruby_info.lib, ruby_info.share],
        command = ctx.expand_location(_BUILD_ARCHIVE.format(
            hermetic_tar_setup = _HERMETIC_TAR,
            toolchain = ruby_info.toolchain,
            output = archive.path,
        )),
    )

    return [DefaultInfo(files = depset([archive]))]

_ruby_archive = rule(
    attrs = {
        "ruby": attr.label(),
    },
    provides = [DefaultInfo],
    implementation = _ruby_archive_impl,
)

_BINARY_WRAPPER = """#!/bin/bash

{runfiles_setup}

binary_path="$(rlocation {workspace}/toolchain/bin/{binary})"

export PATH="$(dirname "$binary_path"):$PATH"

exec "$binary_path" "$@"
"""

def _ruby_binary_impl(ctx):
    workspace = ctx.label.workspace_name

    ruby_info = ctx.attr.ruby[RubyInfo]

    wrapper = ctx.actions.declare_file(ctx.label.name)

    binary = None

    for candidate in ruby_info.binaries:
        if candidate.basename == ctx.label.name:
            binary = candidate
            break

    if binary == None:
        fail("Binary '{}' is missing from the ruby build".format(ctx.label.name))

    ctx.actions.write(
        output = wrapper,
        content = _BINARY_WRAPPER.format(
            runfiles_setup = _RUNFILES_BASH,
            workspace = workspace,
            binary = ctx.label.name,
        ),
        is_executable = True,
    )

    runfiles_bash = ctx.attr._runfiles_bash[DefaultInfo].default_runfiles

    symlinks = {}

    symlinks["{}/toolchain/include".format(workspace)] = ruby_info.includes
    symlinks["{}/toolchain/lib".format(workspace)] = ruby_info.lib
    symlinks["{}/toolchain/share".format(workspace)] = ruby_info.share

    for target in ruby_info.binaries:
        symlinks["{}/toolchain/bin/{}".format(workspace, target.basename)] = target

    runfiles = ctx.runfiles(root_symlinks = symlinks)
    runfiles = runfiles.merge(runfiles_bash)

    return [DefaultInfo(executable = wrapper, runfiles = runfiles)]

_ruby_binary = rule(
    attrs = {
        "ruby": attr.label(),
        "_runfiles_bash": attr.label(
            default = Label("@bazel_tools//tools/bash/runfiles"),
        ),
    },
    executable = True,
    provides = [DefaultInfo],
    implementation = _ruby_binary_impl,
)

def _uses_headers(ctx, paths, headers):
    ruby_info = ctx.attr.ruby[RubyInfo]

    cc_toolchain = find_cpp_toolchain(ctx)

    feature_configuration = cc_common.configure_features(
        ctx = ctx,
        cc_toolchain = cc_toolchain,
        requested_features = ctx.features,
        unsupported_features = ctx.disabled_features,
    )

    compilation_context = cc_common.create_compilation_context(
        includes = depset(paths),
        headers = headers,
    )

    return [
        DefaultInfo(files = headers),
        CcInfo(compilation_context = compilation_context),
    ]

def _ruby_headers_impl(ctx):
    ruby_info = ctx.attr.ruby[RubyInfo]

    paths = [
        ruby_info.includes.path,
        "{}/ruby-arch".format(ruby_info.includes.path),
    ]

    headers = depset([ruby_info.includes])

    return _uses_headers(ctx, paths, headers)

_ruby_headers = rule(
    attrs = {
        "ruby": attr.label(),
        "_cc_toolchain": attr.label(
            default = Label("@bazel_tools//tools/cpp:current_cc_toolchain"),
        ),
    },
    fragments = ["cpp"],
    provides = [
        DefaultInfo,
        CcInfo,
    ],
    toolchains = ["@bazel_tools//tools/cpp:toolchain_type"],
    implementation = _ruby_headers_impl,
)

def _ruby_internal_headers_impl(ctx):
    ruby_info = ctx.attr.ruby[RubyInfo]

    paths = [ruby_info.internal_includes.path]

    headers = depset([ruby_info.internal_includes])

    return _uses_headers(ctx, paths, headers)

_ruby_internal_headers = rule(
    attrs = {
        "ruby": attr.label(),
        "_cc_toolchain": attr.label(
            default = Label("@bazel_tools//tools/cpp:current_cc_toolchain"),
        ),
    },
    fragments = ["cpp"],
    provides = [
        DefaultInfo,
        CcInfo,
    ],
    toolchains = ["@bazel_tools//tools/cpp:toolchain_type"],
    implementation = _ruby_internal_headers_impl,
)

def ruby(rubygems, gems, extra_srcs = None, append_srcs = None, configure_flags = [], copts = [], cppopts = [], linkopts = [], deps = []):
    """
    Define a ruby build.
    """

    native.filegroup(
        name = "source",
        srcs = native.glob(["**/*"]),
        visibility = ["//visibility:private"],
    )

    _build_ruby(
        name = "ruby-dist",
        src = ":source",
        extra_srcs = extra_srcs,
        append_srcs = append_srcs,
        rubygems = rubygems,
        configure_flags = configure_flags,
        copts = copts,
        cppopts = cppopts,
        linkopts = linkopts,
        deps = deps,
        gems = gems,
        # This is a hack because macOS Catalina changed the way that system headers and libraries work.
        sysroot_flag = select({
            "@platforms//os:osx": "-isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk",
            "//conditions:default": "",
        }),
    )

    _ruby_headers(
        name = "headers",
        ruby = ":ruby-dist",
        visibility = ["//visibility:public"],
    )

    _ruby_internal_headers(
        name = "headers-internal",
        ruby = ":ruby-dist",
        visibility = ["//visibility:public"],
    )

    _ruby_archive(
        name = "ruby.tar.gz",
        ruby = ":ruby-dist",
        visibility = ["//visibility:public"],
    )

    _ruby_binary(
        name = "ruby",
        ruby = ":ruby-dist",
        visibility = ["//visibility:public"],
    )

    _ruby_binary(
        name = "irb",
        ruby = ":ruby-dist",
        visibility = ["//visibility:public"],
    )

    _ruby_binary(
        name = "gem",
        ruby = ":ruby-dist",
        visibility = ["//visibility:public"],
    )

    _ruby_binary(
        name = "bundler",
        ruby = ":ruby-dist",
        visibility = ["//visibility:public"],
    )

    _ruby_binary(
        name = "bundle",
        ruby = ":ruby-dist",
        visibility = ["//visibility:public"],
    )
