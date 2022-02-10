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
    1.times do | end
    1.times do | puts 'hello' end
    1.times do | puts('hello') end
    1.times do | x = nil end
    1.times do | Opus::Log.info end
  end

  def test_one_arg
    1.times do |x end
    1.times do |x puts 'hello' end
    1.times do |x puts('hello') end
    1.times do |x x = nil end
    1.times do |x Opus::Log.info end
  end

  def test_one_arg_comma
    1.times do |x, end
    # 1.times do |x, puts 'hello' end
    1.times do |x, puts('hello') end
    1.times do |x, x = nil end
    1.times do |x, Opus::Log.info end
  end

  def test_two_args
    1.times do |x, y end
    # 1.times do |x, y puts 'hello' end
    # 1.times do |x, y puts('hello') end
    # 1.times do |x, y x = nil end
    # 1.times do |x, y Opus::Log.info end
  end
end
