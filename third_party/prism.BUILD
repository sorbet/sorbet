GENERATED_SRCS = [
  "src/diagnostic.c",
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
    export PATH="$(RUBY_PATH)/bin:$$PATH"

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
  srcs = glob(["src/**/*.c"], exclude=GENERATED_SRCS) + [":generate_templates"],
  hdrs = glob(["include/**/*.h"], exclude=GENERATED_HDRS) + [":generate_templates"],
  visibility = ["//visibility:public"],
  includes = ["include"],
  copts = ["-Wno-implicit-fallthrough"],
)
