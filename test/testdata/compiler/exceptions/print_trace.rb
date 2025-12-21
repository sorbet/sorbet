# frozen_string_literal: true
# typed: true
# compiled: true

# Tests the content of the backtrace from the exception. Because of differences
# between where the code is run, we need to edit the backtrace lines in order to
# get the test to pass. This is the same as how
# test/testdata/compiler/backtrace.rb is handled.

begin
  raise "foo"
rescue => e
  path = Dir.getwd + '/'
  T.must(e.backtrace).each do |line|
    puts line
      .sub(path, '')
      # Strip bazel cache paths (compiled code uses absolute paths)
      .sub(%r{/root/\.cache/bazel/_bazel_root/[^/]+/execroot/com_stripe_ruby_typer/}, '')
      .sub(%r{.*test/patch_require.rb:.*}, 'test/patch_require.rb:<censored>:')
      .sub(%r{^.*tmp\..*:}, '<censored>') # OSX
      .sub(%r{^/tmp.*:}, '<censored>')    # linux
  end
end
