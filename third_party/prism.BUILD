load("@ruby_root//:ruby_root.bzl", "RUBY_ROOT") # Get the RUBY_ROOT environment variable

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
  srcs = [
    "templates/template.rb",
    "prism.gemspec",
    "Gemfile",
    "Gemfile.lock",
    "config.yml", # Contains the data to populate the ERB templates.
  ] + \
  ["templates/{c_file}.erb".format(c_file = c_file) for c_file in GENERATED_SRCS] + \
  ["templates/{h_file}.erb".format(h_file = h_file) for h_file in GENERATED_HDRS],
  outs = GENERATED_HDRS + GENERATED_SRCS,
  cmd = """
    # set -o xtrace # Uncomment this to debug the execution of this script.
    # echo "PWD: $$PWD"
    # echo "RULEDIR: $(RULEDIR)"

    export PATH="{ruby_root}/bin:$$PATH"

    gemfile="$(location Gemfile)"
    script="$(location templates/template.rb)"

    bundle install --gemfile="$$gemfile"

    {template_render_commands}

  """.format(
    ruby_root = RUBY_ROOT,
    template_render_commands = "\n    ".join([
      """
        bundle exec --gemfile="$$gemfile" ruby "$$script" {f} "$(location {f})"
      """.format(f=f) for f in (GENERATED_SRCS + GENERATED_HDRS)
    ])
  ),
)

cc_library(
  name = "prism",
  srcs = glob(["src/**/*.c"], exclude=GENERATED_SRCS) + [":generate_templates"],
  hdrs = glob(["include/**/*.h"], exclude=GENERATED_HDRS) + [":generate_templates"],
  visibility = ["//visibility:public"],
  includes = ["include"],
  copts = ["-Wno-implicit-fallthrough"],
)
