# frozen_string_literal: true
# typed: true
# compiled: true
def foo
  bar
end

def bar
  path = Dir.getwd + '/'

  Thread.current.backtrace.map do |line|
    line
      .sub(path, '')
      .sub(%r{^/.*bazel-out/([^/]+)/bin/external/sorbet_ruby_2_7_for_compiler/}, 'bazel-out/\1/bin/external/sorbet_ruby_2_7_for_compiler/')
      .sub(%r{.*test/patch_require.rb:.*:}, 'test/patch_require.rb:<censored>:')
      .sub(%r{^.*tmp\..*:}, '<censored>') # OSX
      .sub(%r{^/tmp.*:}, '<censored>')    # linux
  end
end

puts foo
