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
      .sub(%r{.*run/tools/patch_require.rb:.*:}, 'run/tools/patch_require.rb:<censored>:')
      .sub(%r{^.*tmp\..*:}, '<censored>') # OSX
      .sub(%r{^/tmp.*:}, '<censored>')    # linux
  end
end

puts foo
