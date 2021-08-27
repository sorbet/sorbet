# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: LOWERED
# run_filecheck: OPT

class Tests
  extend T::Sig

  sig(:final) { params(x: Integer, y: Integer).void }
  def self.bar(x, y)
  end

  sig(:final) { params(x: String).void }
  def self.bar2(x)
  end

# LOWERED-LABEL: define i64 @"func_Hello#foo"
# LOWERED: tail call i64 (i64, ...) @sorbet_i_allTypeTested(i64 %rawArg_b, i64 %rawArg_b)
# LOWERED{LITERAL}: }

# OPT-LABEL: define i64 @"func_Hello#foo"
# OPT: tail call %struct.rb_control_frame_struct* @sorbet_pushCfuncFrame(i1 zeroext true
# OPT{LITERAL}: }
  def self.test1(a, b)
    y = T.let(a, Integer)
    if b
      bar(y, a)
    else
      z = T.let(a, Integer)
    end
  end

# LOWERED-LABEL: define i64 @"func_Hello#foo2"
# LOWERED: tail call i64 (i64, ...) @sorbet_i_allTypeTested
# LOWERED{LITERAL}: }

# OPT-LABEL: define i64 @"func_Hello#foo2"
# OPT: tail call %struct.rb_control_frame_struct* @sorbet_pushCfuncFrame(i1 zeroext false
# OPT{LITERAL}: }
  def self.test2(a, b, c)
    y = T.let(b, Integer)
    d = T.unsafe(y)
    bar2 d
  end

# LOWERED-LABEL: define i64 @"func_Hello#baz"
# LOWERED: tail call i64 (i64, ...) @sorbet_i_allTypeTested(i64 %rawArg_a, i64 %rawArg_b)
# LOWERED{LITERAL}: }

# OPT-LABEL: define i64 @"func_Hello#baz"
# OPT: tail call %struct.rb_control_frame_struct* @sorbet_pushCfuncFrame(i1 zeroext false
# OPT{LITERAL}: }
  def self.test3(a, b, c)
    if c
      bar a, b
    else
      z = T.let(b, Integer)
    end
  end
end

Tests.test1(10, true)
