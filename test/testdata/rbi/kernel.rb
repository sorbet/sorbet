# typed: true

T.assert_type!(caller, T::Array[String])
T.assert_type!(caller(10), T.nilable(T::Array[String]))

T.assert_type!(BigDecimal('123', 1, exception: true), BigDecimal)
T.assert_type!(Complex('123', exception: false), Complex)
T.assert_type!(Float('123.0'), Float)
T.assert_type!(Integer('123', exception: true), Integer)
T.assert_type!(Rational('1/1', 2, exception: true), Rational)

# rand
T.assert_type!(Kernel.rand, Float)
T.assert_type!(Kernel.rand(nil), Float)
T.assert_type!(Kernel.rand(1), Integer)
T.assert_type!(Kernel.rand(1.0), Float)
T.assert_type!(Kernel.rand(1..1), Integer)
T.assert_type!(Kernel.rand(1.0..1.0), Float)
T.assert_type!(Kernel.rand(36**8), Numeric)
T.assert_type!(Kernel.rand((36**8..36**9)), Numeric)

# make sure we don't regress and mark `loop` as returning `nil`
x = loop {break 1}
T.assert_type!(x, Integer)
if x
  puts x + 1
end

define_singleton_method(:foo) { puts '' }
define_singleton_method('foo') { puts '' }

def raise_test
  raise ArgumentError, 'bad argument', nil
end

# make sure we don't regress and mark these as errors
env = {'VAR' => 'VAL'}
system('echo')
system('echo', err: File::NULL)
system('echo', out: :err)
system('echo', 'hello')
system('echo', 'hello', err: File::NULL)
system('echo', 'hello', out: :err)
system(['echo', 'echo'])
system(['echo', 'echo'], 'hello')
system(['echo', 'echo'], out: :err)
system(['echo', 'echo'], 'hello', out: :err)
system(env, 'echo')
system(env, 'echo', out: :err)
system(env, ['echo', 'echo'])
system(env, ['echo', 'echo'], out: :err)
system(env, ['echo', 'echo'], 'hello')
system(env, ['echo', 'echo'], 'hello', out: :err)

# then
obj = T.let("foo", String)
# Object#then, with a block
T.reveal_type(obj.then(&:to_i)) # error: Revealed type: `T.untyped`

# object_id
obj = T.let("foo", String)
T.assert_type!(obj.object_id, Integer)

# itself
obj = T.let("foo", String)
T.assert_type!(obj.itself, String)

y = loop do
end
puts y # error: This code is unreachable

class CustomError < StandardError
  def initialize(cause, team)
    @cause = cause
    @team = team
  end
end

def raises_fail
  fail CustomError.new("Problem", "DevOops")
end

def raises_raise
  raise CustomError.new("Problem", "DevOops")
end

def fail_class_message
  fail StandardError, "message"
end
