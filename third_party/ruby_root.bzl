# This rule is used to get the RUBY_ROOT environment variable from the
# user's local environment and make it available to the build system.
def _ruby_root_impl(repository_ctx):
  # Retrieve the RUBY_ROOT environment variable
  ruby_root = repository_ctx.os.environ.get("RUBY_ROOT")

  if not ruby_root:
    fail("RUBY_ROOT environment variable is not set.")

  # Create a file to store the RUBY_ROOT value
  content = 'RUBY_ROOT = "{}"\n'.format(ruby_root)
  repository_ctx.file("ruby_root.bzl", content=content)

  # Create an empty BUILD file to make the directory a valid Bazel repository
  repository_ctx.file("BUILD", content="")

ruby_root = repository_rule(
  implementation = _ruby_root_impl,
  attrs = {},
)
