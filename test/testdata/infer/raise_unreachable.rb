# typed: true
extend T::Sig

sig {params(x: T.any(Integer, String)).void}
def raise_absurd(x)
  case x
  when Integer, String
  else
    raise
    T.absurd(x)
  end
end

sig {params(x: T.any(Integer, String)).void}
def defensive_raise(x)
  case x
  when Integer, String
  else
    raise 'unexpected value'
  # ^^^^^^^^^^^^^^^^^^^^^^^^ error: This code is unreachable
  end
end

sig {params(x: T.any(Integer, String)).void}
def fail_absurd(x)
  case x
  when Integer, String
  else
    fail
    T.absurd(x)
  end
end

sig {params(x: T.any(Integer, String)).void}
def non_exhaustive(x)
  case x
  when Integer
  else
    T.absurd(x)
  # ^^^^^^^^^^^ error: Control flow could reach `T.absurd` because the type `String` wasn't handled
  end
end

sig {params(x: T.any(Integer, String)).void}
def ordinary_dead_code(x)
  case x
  when Integer, String
  else
    puts x
  # ^^^^^^ error: This code is unreachable
  end
end

sig {params(x: T.any(Integer, String)).void}
def separate_raise(x)
  case x
  when Integer, String
  else
    T.absurd(x)
    raise 'unexpected value'
        # ^^^^^^^^^^^^^^^^^^ error: This code is unreachable
  end
end

sig {params(x: T.any(Integer, String)).void}
def code_after_raise_absurd(x)
  case x
  when Integer, String
  else
    raise 'unexpected value'
    # error: This code is unreachable
    puts 'later'
  end
end

sig {params(x: String).void}
def unnecessary_safe_navigation(x)
  x&.upcase
 # ^^ error: Used `&.` operator on `String`, which can never be nil
end

sig {params(x: T.any(Integer, String)).void}
def unrelated_absurd(x)
  case x
  when Integer, String
  else
    raise 'unexpected value'
    T.absurd(x)
  end
end

sig {params(x: T.any(Integer, String)).void}
def other_wrapper(x)
  case x
  when Integer, String
  else
    puts T.absurd(x)
  # ^^^^^^^^^^^^^^^^ error: This code is unreachable
  end
end

# A single predecessor retains its declared receiver and argument types.
sig {params(x: String).void}
def single_predecessor(x)
  if x.is_a?(String)
  else
    raise "unexpected #{x}"
  # ^^^^^^^^^^^^^^^^^^^^^^^ error: This code is unreachable
  end
end

sig {params(x: T.any(Integer, String)).void}
def multiple_predecessors(x)
  case x
  when Integer, String
  else
    puts "unexpected #{x}"
    # error: This code is unreachable
    fail RuntimeError, 'unexpected value'
  end
end

sig {params(x: String).void}
def invalid_raise_argument(x)
  if x.is_a?(String)
  else
    raise 123
        # ^^^ error: Expected `T.any(T::Class[T.anything], Exception, String)` but found `Integer(123)` for argument `arg0`
  end
end

class ReturningRaise
  extend T::Sig

  sig {params(x: String).returns(Integer)}
  def raise(x)
    1
  end

  sig {params(x: String).void}
  def test(x)
    if x.is_a?(String)
    else
      raise(x)
    # ^^^^^^^^ error: This code is unreachable
    end
  end

  sig {params(x: String).void}
  def loop_after_terminating_call(x)
    panic(x)
    while raise(x)
        # ^^^^^^^^ error: This code is unreachable
      puts x
    end
  end

end

sig {params(x: String).void}
def code_after_defensive_raise(x)
  if x.is_a?(String)
  else
    raise "unexpected #{x}"
    # error: This code is unreachable
    puts 'later'
  end
end

sig {params(x: String, flag: T::Boolean).void}
def conditional_message(x, flag)
  if x.is_a?(String)
  else
    raise(flag ? "unexpected #{x}" : 'unexpected value')
  end
end

sig {params(x: String, flag: T::Boolean).void}
def invalid_conditional_message(x, flag)
  if x.is_a?(String)
  else
    # Argument evaluation spans successor blocks, which retain the existing
    # skip behavior once every incoming path is dead.
    raise(flag ? x.length : 123)
  end
end

sig {params(x: String).returns(String)}
def live_join(x)
  result = if x.is_a?(String)
    'handled'
  else
    raise "unexpected #{x}"
  # ^^^^^^^^^^^^^^^^^^^^^^^ error: This code is unreachable
  end
  T.reveal_type(result)
# ^^^^^^^^^^^^^^^^^^^^^ error: Revealed type: `String("handled")`
  result
end

sig {params(message: String).returns(T.noreturn)}
def panic(message)
  raise message
end

sig {params(x: T.any(Integer, String)).void}
def custom_guard(x)
  case x
  when Integer, String
  else
    value = x
    # error: This code is unreachable
    puts "unexpected #{value}"
    panic("unexpected #{value}")
  end
end

sig {params(x: String).void}
def repeated_guard(x)
  unless true
    panic(x)
    panic(x)
  # ^^^^^^^^ error: This code is unreachable
    T.absurd(x)
  end
end

sig {params(x: String).void}
def dead_absurd_after_guard(x)
  unless true
    panic(x)
    T.absurd(x)
  end
end

sig {params(x: String).void}
def invalid_custom_guard_argument(x)
  unless true
    panic(123)
  # ^^^^^^^^^^ error: This code is unreachable
    # ^^^^^ hover: NilClass
  end
end

sig {params(x: String).void}
def unrelated_dead_absurd(x)
  unless true
    T.absurd(x)
  end
end

sig {params(x: String).void}
def dead_return(x)
  unless true
    return x
  # ^^^^^^^^ error: This code is unreachable
  end
end

sig {params(x: String).void}
def untyped_guard(x)
  unless true
    T.unsafe(self).panic(x)
  # ^^^^^^^^^^^^^^^^^^^^^^^ error: This code is unreachable
  end
end

sig {params(x: T.any(Integer, String)).void}
def guard_does_not_assert_exhaustiveness(x)
  case x
  when Integer
  else
    panic('unhandled string')
    T.absurd(x)
  end
end

sig {params(x: String).void}
def error_in_guard_body(x)
  unless true
    x.not_a_method
    # error: This code is unreachable
    panic(x)
  end
end

# Unpaired dead blocks retain their existing skip behavior.
sig {params(x: String).void}
def rejected_guard_diagnostics(x)
  unless true
    T.reveal_type(x)
    # error: This code is unreachable
    x.not_a_method
  end
end

sig {params(x: String).void}
def intrinsic_before_unpaired_guard(x)
  unless true
    T.reveal_type(x)
    # error: This code is unreachable
    panic(x)
  end
end

class GuardBase
  extend T::Sig

  sig {params(message: String).returns(T.noreturn)}
  def stop(message)
    raise message
  end
end

class InheritedGuard < GuardBase
  extend T::Sig

  sig {params(x: String).void}
  def test(x)
    unless true
      stop(x)
    # ^^^^^^^ error: This code is unreachable
    end
  end
end

sig {params(x: String, guard: GuardBase).void}
def explicit_receiver(x, guard)
  unless true
    guard.stop(x)
  # ^^^^^^^^^^^^^ error: This code is unreachable
    #          ^ hover: NilClass
  end
end

class ReturningGuard
  extend T::Sig

  sig {params(message: String).void}
  def stop(message); end
end

sig {params(x: String, guard: T.any(GuardBase, ReturningGuard)).void}
def possibly_returning_guard(x, guard)
  unless true
    guard.stop(x)
  # ^^^^^^^^^^^^^ error: This code is unreachable
  end
end

# Without T.absurd, even a custom noreturn helper leaves this block unreachable.
sig {params(x: String).void}
def bottom_local_before_guard(x)
  unless true
    impossible = BottomValues::VALUE
    # error: This code is unreachable
    copy = impossible
    panic(copy)
  end
end

sig {params(x: String).void}
def bottom_constant_is_not_a_guard(x)
  unless true
    BottomValues::VALUE
  # ^^^^^^^^^^^^^^^^^^^ error: This code is unreachable
  end
end

# A bottom cast does not excuse a dead block without T.absurd.
sig {params(x: String).void}
def bottom_cast_without_absurd(x)
  unless true
    T.cast(T.unsafe(x), T.noreturn)
    # error: This code is unreachable
    panic(x)
  end
end

# Invalid dispatch still reports its error and does not infer a bottom result.
sig {params(x: String).void}
def bottom_receiver_is_not_a_guard(x)
  unless true
    BottomValues::VALUE.upcase
    # error: This code is unreachable
    puts x
  end
end

sig {params(x: String).void}
def bottom_intrinsic_without_absurd(x)
  unless true
    T.reveal_type(BottomValues::VALUE)
    # error: This code is unreachable
    puts x
  end
end

sig {type_parameters(:U).params(value: T.type_parameter(:U)).returns(T.type_parameter(:U))}
def identity(value)
  value
end

sig {params(x: String).void}
def bottom_generic_without_absurd(x)
  unless true
    identity(BottomValues::VALUE)
    # error: This code is unreachable
    puts x
  end
end

# Bottom constants exercise the normal reachability rule for reads.
module BottomValues
  VALUE = T.let(T.unsafe(nil), T.noreturn)
                             # ^ error: This code is unreachable
end

# Values do not excuse a dead branch, even when retained as a block's result.
sig {void}
def literal_is_not_a_guard
  unless true
    123
  # ^^^ error: This code is unreachable
  end
end

sig {void}
def self_is_not_a_guard
  unless true
    self
  # ^^^^ error: This code is unreachable
  end
end

# Keep the literal live as the guard's argument rather than an unused assignment.
sig {void}
def literal_before_guard
  unless true
    message = 'unexpected value' # error: This code is unreachable
    panic(message)
  end
end

# A bottom argument does not replace the requirement for T.absurd.
sig {params(value: T.noreturn).void}
def bottom_argument_is_not_a_guard(value)
  unless true
       # ^^^^ error: This code is unreachable
    value
  end
end

# Control transfers have bottom results, but cannot excuse a dead branch.
sig {void}
def return_is_not_a_guard
  unless true
    return
  # ^^^^^^ error: This code is unreachable
  end
end

sig {void}
def block_return_is_not_a_guard
  [1].each do
    unless true
      next
    # ^^^^ error: This code is unreachable
    end
  end
end

sig {type_parameters(:U).params(blk: T.proc.returns(T.type_parameter(:U))).returns(T.type_parameter(:U))}
def result_from_block(&blk)
  yield
end

# SolveConstraint completes the call's result after its block has been checked.
sig {void}
def bottom_call_result_after_block
  result_from_block do
    raise 'stop'
  end
  puts 'later'
     # ^^^^^^^ error: This code is unreachable
end

# A call returning bottom does not excuse a dead-entry block without T.absurd,
# including when SolveConstraint determines its result in a successor block.
sig {void}
def dead_call_with_block
  unless true
    result_from_block do # error: This code is unreachable
      raise 'stop'
    end
  end
end

sig {params(blk: T.proc.params(value: T.noreturn).void).void}
def supplies_bottom(&blk)
end

# Loading a bottom block parameter retains the existing entry reachability rule.
sig {void}
def bottom_block_argument_is_not_a_guard
  supplies_bottom do |value|
    unless true
         # ^^^^ error: This code is unreachable
      value
    end
  end
end

sig {params(blk: T.proc.bind(T.noreturn).void).void}
def supplies_bottom_self(&blk)
end

# A bottom self binding also uses the existing block-entry reachability rule.
sig {void}
def bottom_block_self_is_not_a_guard
  supplies_bottom_self do
    self
  # ^^^^ error: This code is unreachable
  end
end

# Preparation may contain arbitrary sends, including ones that return normally.
sig {params(bar: String).void}
def prepared_raise_with_absurd(bar)
  if bar.is_a?(String)
  else
    msg = "foo #{bar}"
    raise msg + " " + msg.size.to_s
    T.absurd(bar)
  end
end

sig {params(bar: String).void}
def prepared_raise_without_absurd(bar)
  if bar.is_a?(String)
  else
    msg = "foo #{bar}" # error: This code is unreachable
    raise msg + " " + msg.size.to_s
  end
end

sig {params(bar: String).void}
def returning_send_with_absurd(bar)
  if bar.is_a?(String)
  else
    puts "unexpected #{bar}"
    T.absurd(bar)
  end
end

# The exemption does not skip type checking the preparation or guard arguments.
sig {params(bar: String).void}
def invalid_preparation_with_absurd(bar)
  if bar.is_a?(String)
  else
    panic(123)
        # ^^^ error: Expected `String` but found `Integer(123)` for argument `message`
    T.absurd(bar)
  end
end

sig {params(bar: String).void}
def code_between_raise_and_absurd(bar)
  if bar.is_a?(String)
  else
    raise 'unexpected'
    puts bar
  # ^^^^^^^^ error: This code is unreachable
    T.absurd(bar)
  end
end

sig {params(bar: String).void}
def code_after_absurd(bar)
  if bar.is_a?(String)
  else
    puts bar
    T.absurd(bar)
    puts 'later'
       # ^^^^^^^ error: This code is unreachable
  end
end

# The name and return type of the preceding send are irrelevant to the exemption.
sig {params(bar: String).void}
def returning_raise_with_absurd(bar)
  if bar.is_a?(String)
  else
    ReturningRaise.new.raise(bar)
    T.absurd(bar)
  end
end

sig {params(bar: String).void}
def untyped_send_with_absurd(bar)
  if bar.is_a?(String)
  else
    T.unsafe(self).panic(bar)
    T.absurd(bar)
  end
end

# The certificate must be in this basic block, not an all-dead successor.
sig {params(bar: String, flag: T::Boolean).void}
def absurd_in_successor(bar, flag)
  if bar.is_a?(String)
  else
    puts bar # error: This code is unreachable
    if flag
      T.absurd(bar)
    end
  end
end

# Bottom reads follow normal reachability rules in a dead block with T.absurd.
sig {params(bar: String).void}
def bottom_read_with_absurd(bar)
  unless true
    impossible = BottomValues::VALUE
               # ^^^^^^^^^^^^ error: This code is unreachable
    panic(impossible)
    T.absurd(bar)
  end
end
