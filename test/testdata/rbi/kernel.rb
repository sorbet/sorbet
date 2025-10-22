# typed: true

extend T::Sig

T.assert_type!(caller, T::Array[String])
T.assert_type!(caller(10), T.nilable(T::Array[String]))

T.assert_type!(caller_locations, T::Array[Thread::Backtrace::Location])
T.assert_type!(caller_locations(2, 10), T.nilable(T::Array[Thread::Backtrace::Location]))
T.assert_type!(caller_locations(1..10), T.nilable(T::Array[Thread::Backtrace::Location]))

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

sig { params(obj_or_str: T.any(Exception, String)).void }
def raise_obj_or_string(obj_or_str)
  raise obj_or_str
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

p_result = p
# Returns nil when called with no arguments
T.reveal_type(p_result) # error: Revealed type: `NilClass`

p_result = Kernel.p 1
# Returns the argument when called with one argument
T.reveal_type(p_result) # error: Revealed type: `Integer`

p_result = p "string"
# Returns the argument when called with one argument
T.reveal_type(p_result) # error: Revealed type: `String`

p_result = p 1, 2
# Returns an array when called with multiple arguments
T.reveal_type(p_result) # error: Revealed type: `T::Array[Integer]`

p_result = Kernel.p 1, "string"
# Returns a union type array when called with multiple arguments
T.reveal_type(p_result) # error: Revealed type: `T::Array[T.any(Integer, String)]`

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

y = loop do
end
puts y # error: This code is unreachable

class Test
  def test
    a = 1
    b = 2
  end
end

set_trace_func proc { |event, file, line, id, binding, classname|
    printf "%8s %s:%-2d %10s %8s\n", event, file, line, id, classname
}
t = Test.new
t.test

set_trace_func(nil)

require "continuation"
callcc {|cont|
  for i in 0..4
    print "\n#{i}: "
    for j in i*5...(i+1)*5
      cont.call() if j == 17
      printf "%3d", j
    end
  end
}
puts
