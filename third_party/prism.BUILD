GENERATED_SRCS = [
  "src/node.c",
  "src/prettyprint.c",
  "src/serialize.c",
  "src/token_type.c",
]

GENERATED_HDRS = [
  "include/prism/ast.h",
  "include/prism/diagnostic.h",
]

genrule(
  name = "generate_templates",
  srcs = ["templates/template.rb"],
  outs = GENERATED_HDRS + GENERATED_SRCS,
  cmd = """
    # This is a workaround; without guidance, Bazel will try to use the system Ruby,
    # which is too old to install gems and run this script.
    #
    # Pass the RUBY_PATH variable to the build using the --define flag, e.g.
    # ./bazel build //main:sorbet --config=dbg --define RUBY_PATH=/path/to/ruby
    export PATH="$(RUBY_PATH):$$PATH"

    cd external/prism

    bundle install
    bundle exec ruby templates/template.rb

    cd ../..
  """ + " && ".join([
    "cp $$PWD/external/prism/{f} $(RULEDIR)/{f}".format(f=f) for f in GENERATED_HDRS + GENERATED_SRCS
  ]),
)

cc_library(
  name = "prism",
  srcs = [
    "src/encoding.c",
    "src/options.c",
    "src/pack.c",
    "src/prism.c",
    "src/regexp.c",
    "src/static_literals.c",
    "src/util/pm_buffer.c",
    "src/util/pm_char.c",
    "src/util/pm_constant_pool.c",
    "src/util/pm_integer.c",
    "src/util/pm_list.c",
    "src/util/pm_memchr.c",
    "src/util/pm_newline_list.c",
    "src/util/pm_string.c",
    "src/util/pm_string_list.c",
    "src/util/pm_strncasecmp.c",
    "src/util/pm_strpbrk.c",
  ] + GENERATED_SRCS,
  hdrs = [
    "include/prism/defines.h",
    "include/prism/encoding.h",
    "include/prism/node.h",
    "include/prism/options.h",
    "include/prism/pack.h",
    "include/prism/parser.h",
    "include/prism/prettyprint.h",
    "include/prism/regexp.h",
    "include/prism/static_literals.h",
    "include/prism/version.h",
    "include/prism/util/pm_buffer.h",
    "include/prism/util/pm_char.h",
    "include/prism/util/pm_constant_pool.h",
    "include/prism/util/pm_integer.h",
    "include/prism/util/pm_list.h",
    "include/prism/util/pm_memchr.h",
    "include/prism/util/pm_newline_list.h",
    "include/prism/util/pm_string.h",
    "include/prism/util/pm_string_list.h",
    "include/prism/util/pm_strncasecmp.h",
    "include/prism/util/pm_strpbrk.h",
  ] + GENERATED_HDRS,
  visibility = ["//visibility:public"],
  includes = ["include"],
  copts = ["-Wno-implicit-fallthrough"],
)
