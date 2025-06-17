# frozen_string_literal: true
# typed: true
# compiled: true
def foo
  bar
end

def bar
  path = Dir.getwd + '/'

  T.must(Thread.current.backtrace).map do |line|
    line
      .sub(path, '')
      .sub(%r{.*test/patch_require.rb:.*:}, 'test/patch_require.rb:<censored>:')
      .sub(%r{^.*tmp\..*:}, '<censored>') # OSX
      .sub(%r{^/tmp.*:}, '<censored>')    # linux
  end
end

puts foo
