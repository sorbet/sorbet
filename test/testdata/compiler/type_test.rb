# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: LOWERED
# run_filecheck: OPT

class Hello
  extend T::Sig

  sig(:final) { params(x: Integer, y: Integer).void }
  def bar(x, y)
  end

# LOWERED-LABEL: define i64 @"func_Hello#foo"
# LOWERED: tail call i64 (i64, ...) @sorbet_i_all_type_tested(i64 %rawArg_b, i64 %rawArg_b)
# LOWERED{LITERAL}: }

# OPT-LABEL: define i64 @"func_Hello#foo"
# OPT: tail call %struct.rb_control_frame_struct* @sorbet_pushCfuncFrame(i1 zeroext true
# OPT{LITERAL}: }
  def foo(a, b, c)
    y = T.let(b, Integer)
    if c
      bar y, b
    else
      z = T.let(b, Integer)
    end
  end

# LOWERED-LABEL: define i64 @"func_Hello#baz"
# LOWERED: tail call i64 (i64, ...) @sorbet_i_all_type_tested(i64 %rawArg_a, i64 %rawArg_b)
# LOWERED{LITERAL}: }

# OPT-LABEL: define i64 @"func_Hello#baz"
# OPT: tail call %struct.rb_control_frame_struct* @sorbet_pushCfuncFrame(i1 zeroext false
# OPT{LITERAL}: }
  def baz(a, b, c)
    if c
      bar a, b
    else
      z = T.let(b, Integer)
    end
  end
end
