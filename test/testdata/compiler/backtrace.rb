# frozen_string_literal: true
# typed: true
# compiled: true
def foo
  bar
end

def bar
  path = Dir.getwd + '/'

  Thread.current.backtrace.map do |line|
    if line.start_with?('/tmp') then
      nil
    else
      line
        .sub(path, '')
        .sub(%r{.*run/tools/patch_require.rb:.*:}, 'run/tools/patch_require.rb:<censored>:')
    end
  end.compact
end

puts foo
