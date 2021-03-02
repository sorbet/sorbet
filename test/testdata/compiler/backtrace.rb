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
      .sub('/usr/local/var/bazelcache/output-bases/test/execroot/com_stripe_sorbet_llvm/', '') # ... buildkite? idk https://buildkite.com/sorbet/sorbet-compiler/builds/3487
      .sub(%r{.*test/patch_require.rb:.*:}, 'test/patch_require.rb:<censored>:')
      .sub(%r{^.*tmp\..*:}, '<censored>') # OSX
      .sub(%r{^/tmp.*:}, '<censored>')    # linux
  end
end

puts foo
