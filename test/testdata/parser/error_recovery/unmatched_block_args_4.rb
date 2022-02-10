# typed: false

module Opus::Log
  def self.info; end
end

# The commented out lines are the ones that don't parse at the time of writing
# this test. If any of them is uncommented it makes the whole file appear to
# fail to parse at all, but I didn't want to make one file for each of them in
# isolation.

class A
  def test_no_args
    1.times { | }
    1.times { | puts 'hello' }
    1.times { | puts('hello') }
    1.times { | x = nil }
    1.times { | Opus::Log.info }
  end

  def test_one_arg
    1.times { |x }
    1.times { |x puts 'hello' }
    1.times { |x puts('hello') }
    1.times { |x x = nil }
    1.times { |x Opus::Log.info }
  end

  def test_one_arg_comma
    1.times { |x, }
    # 1.times { |x, puts 'hello' }
    1.times { |x, puts('hello') }
    1.times { |x, x = nil }
    1.times { |x, Opus::Log.info }
  end

  def test_two_args
    1.times { |x, y }
    # 1.times { |x, y puts 'hello' }
    # 1.times { |x, y puts('hello') }
    # 1.times { |x, y x = nil }
    # 1.times { |x, y Opus::Log.info }
  end
end
