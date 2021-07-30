# frozen_string_literal: true
# typed: true
# compiled: true

class A
  def foo; end
end

# Account for different locations of where the interpreted testcases and the
# compiled testcases run.  Compare backtrace.rb for similar twiddling.
loc = T.must(A.instance_method(:foo).source_location)
path = Dir.getwd + '/'
loc[0] = loc[0].sub(path, '')
p loc
