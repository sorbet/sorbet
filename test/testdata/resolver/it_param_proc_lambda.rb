# typed: true

# Test 'it' parameter behavior in proc vs lambda
# Based on Ruby 3.4 spec: proc { it**2 }.parameters #=> [[:opt, nil]]
#                         lambda { it**2 }.parameters #=> [[:req]]

# Proc with 'it' parameter
p1 = proc { it * 2 }
result1 = p1.call(5)

# Lambda with 'it' parameter
l1 = lambda { it * 2 }
result2 = l1.call(5)

# Proc with String parameter
p2 = proc { it.upcase }
result3 = p2.call("hello")

# Lambda with String parameter
l2 = lambda { it.upcase }
result4 = l2.call("hello")

# Type inference through proc/lambda
str_proc = proc { it.length }
num = str_proc.call("test")

# Nested procs with 'it' - each has its own
nested_proc = proc do
  # Outer 'it'
  outer = it

  proc do
    # Inner 'it' - different from outer
    inner = it
    inner * 2
  end
end

# Proc that uses 'it' in multiple operations
complex_proc = proc do
  x = it * 2
  y = it + 3
  z = it - 1
  x + y + z
end

result5 = complex_proc.call(10)

# Lambda with type-specific operations
str_lambda = lambda do
  it.upcase + it.downcase + it.capitalize
end

result6 = str_lambda.call("Ruby")

# Proc that modifies 'it' variable (soft keyword behavior)
mutation_proc = proc do
  orig = it
  it = orig * 2  # Reassigning 'it' is allowed
  it
end

result7 = mutation_proc.call(7)

# Block converted to proc maintains 'it' behavior
def takes_block(&block)
  block.call(100)
end

result8 = takes_block { it + 50 }

# Proc with signature annotations
sig_proc = T.let(
  proc { it * 2 },
  T.proc.params(arg0: Integer).returns(Integer)
)

result9 = sig_proc.call(15)

# Lambda with type errors caught
error_lambda = lambda { it.upcase }
# Calling with wrong type will fail at runtime
# error_lambda.call(123)

# Proc that expects String but gets Integer
string_proc = proc do
  # Assuming 'it' is String
  it.length + it.size
end

int_result = string_proc.call(100)  # Works but 'it' is Integer, not String

# Type error: calling Integer method on what should be String
type_error_proc = proc do
  # Assuming Integer
  it * 2
end
# This would fail if called with String:
# type_error_proc.call("hello") # error at runtime
