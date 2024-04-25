# frozen_string_literal: true
# typed: true
# compiled: true

# The current implementation for default arguments doesn't consider `a` as being
# captured by the defaultArgs function that's extracted to initialize `b`. The
# result is that the compiled version does not update the value of `a` when
# providing the default value for `b`.
def test(a, b=(a = 'test'))
  puts "a = #{a}"
  puts "b = #{b}"
end

test('foo')
test('foo', 'bar')
