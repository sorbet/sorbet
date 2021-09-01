# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: OPT

class Tests
  extend T::Sig

  sig(:final) { params(x: Integer, y: Integer).void }
  def self.bar(x, y)
  end

  sig(:final) { params(x: String).void }
  def self.bar2(x)
  end

# OPT-LABEL: define i64 @func_Tests.test1
# TODO(trevor) we're not adding typeTested assertions on the LHS of a T.let
# OPT: tail call %struct.rb_control_frame_struct* @sorbet_pushCfuncFrame(i1 noundef zeroext true
# OPT{LITERAL}: }
  def self.test1(a, b)
    y = T.let(a, Integer)
    if b
      bar(y, a)
    else
      z = T.let(a, Integer)
    end
  end

# OPT-LABEL: define i64 @func_Tests.test2
# OPT: tail call %struct.rb_control_frame_struct* @sorbet_pushCfuncFrame(i1 noundef zeroext false
# OPT{LITERAL}: }
  def self.test2(a, b, c)
    y = T.let(b, Integer)
    d = T.unsafe(y)
    bar2 d
  end

# OPT-LABEL: define i64 @func_Tests.test3
# OPT: tail call %struct.rb_control_frame_struct* @sorbet_pushCfuncFrame(i1 noundef zeroext false
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
