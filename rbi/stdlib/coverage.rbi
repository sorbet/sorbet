# typed: __STDLIB_INTERNAL

# [`Coverage`](https://docs.ruby-lang.org/en/2.6.0/Coverage.html) provides
# coverage measurement feature for Ruby. This feature is experimental, so these
# APIs may be changed in future.
#
# # Usage
#
# 1.  require "coverage"
# 2.  do
#     [`Coverage.start`](https://docs.ruby-lang.org/en/2.6.0/Coverage.html#method-c-start)
# 3.  require or load Ruby source file
# 4.  [`Coverage.result`](https://docs.ruby-lang.org/en/2.6.0/Coverage.html#method-c-result)
#     will return a hash that contains filename as key and coverage array as
#     value. A coverage array gives, for each line, the number of line execution
#     by the interpreter. A `nil` value means coverage is disabled for this line
#     (lines like `else` and `end`).
#
#
# # Example
#
# ```ruby
# [foo.rb]
# s = 0
# 10.times do |x|
#   s += x
# end
#
# if s == 45
#   p :ok
# else
#   p :ng
# end
# [EOF]
#
# require "coverage"
# Coverage.start
# require "foo.rb"
# p Coverage.result  #=> {"foo.rb"=>[1, 1, 10, nil, nil, 1, 1, nil, 0, nil]}
# ```
module Coverage
  # Returns a hash that contains filename as key and coverage array as value. If
  # `clear` is true, it clears the counters to zero. If `stop` is true, it
  # disables coverage measurement.
  sig {returns(T::Hash[String, T::Array[T.nilable(Integer)]])}
  def self.result(); end

  # Enables coverage measurement.
  sig {returns(NilClass)}
  def self.start(); end
end
