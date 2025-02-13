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
    1.times { | } # parser-error: unmatched "|" in block argument list
    1.times { | puts 'hello' } # parser-error: unmatched "|" in block argument list
    1.times { | puts('hello') } # parser-error: unmatched "|" in block argument list
    1.times { | x = nil } # parser-error: unmatched "|" in block argument list
    1.times { | Opus::Log.info } # parser-error: unmatched "|" in block argument list
  end

  def test_one_arg
    1.times { |x } # parser-error: unmatched "|" in block argument list
    1.times { |x puts 'hello' } # parser-error: unmatched "|" in block argument list
    1.times { |x puts('hello') } # parser-error: unmatched "|" in block argument list
    1.times { |x x = nil } # parser-error: unmatched "|" in block argument list
    1.times { |x Opus::Log.info } # parser-error: unmatched "|" in block argument list
  end

  def test_one_arg_comma
    1.times { |x, } # parser-error: unmatched "|" in block argument list
    # 1.times { |x, puts 'hello' }
    1.times { |x, puts('hello') }
    #         ^ parser-error: unmatched "|" in block argument list
    #                           ^ parser-error: unexpected token tRCURLY
    1.times { |x, x = nil } # parser-error: unmatched "|" in block argument list
    1.times { |x, Opus::Log.info }
    #         ^ parser-error: unmatched "|" in block argument list
    #                            ^ parser-error: unexpected token tRCURLY
  end

  def test_two_args
    1.times { |x, y }
    #         ^ parser-error: unmatched "|" in block argument list
    #               ^ parser-error: unexpected token tRCURLY
    # 1.times { |x, y puts 'hello' }
    # 1.times { |x, y puts('hello') }
    # 1.times { |x, y x = nil }
    # 1.times { |x, y Opus::Log.info }
  end
end
