# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

extend T::Sig

class SomethingElse
  def >=(arg0)
    "got arg: #{arg0}"
  end
end

sig {params(x: T.any(Integer, SomethingElse), y: T.untyped).returns(T.untyped)}
def do_geq(x, y)
  x >= y
end

# INITIAL-LABEL: @"func_Object#6do_geq"
# INITIAL-NOT: call i64 @sorbet_vm_geq
# INITIAL: call i64{{.*}}@sorbet_i_send
# INITIAL-NOT: call i64 @sorbet_vm_geq
# INITIAL: call i64 @sorbet_rb_int_ge
# INITIAL-NOT: call i64 @sorbet_vm_geq
# INITIAL{LITERAL}: }

def do_geq_untyped(x, y)
  x >= y
end

# INITIAL-LABEL: @"func_Object#14do_geq_untyped"
# INITIAL-NOT: call i64 @sorbet_rb_int_ge
# INITIAL: call i64 @sorbet_vm_geq
# INITIAL-NOT: call i64 @sorbet_rb_int_ge
# INITIAL{LITERAL}: }

p do_geq(4, 5)
p do_geq(8, 9.0)
p do_geq(SomethingElse.new, :foo)

p do_geq_untyped(4, 5)
p do_geq_untyped(7, 3.0)
p do_geq_untyped(SomethingElse.new, [:foo])
