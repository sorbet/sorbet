# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

extend T::Sig

class SomethingElse
  def >(arg0)
    "got arg: #{arg0}"
  end
end

sig {params(x: T.any(Integer, SomethingElse), y: T.untyped).returns(T.untyped)}
def do_gt(x, y)
  x > y
end

# INITIAL-LABEL: @"func_Object#5do_gt"
# INITIAL-NOT: call i64 @sorbet_vm_gt
# INITIAL: call i64{{.*}}@sorbet_i_send
# INITIAL-NOT: call i64 @sorbet_vm_gt
# INITIAL: call i64 @sorbet_rb_int_gt
# INITIAL-NOT: call i64 @sorbet_vm_gt
# INITIAL{LITERAL}: }

def do_gt_untyped(x, y)
  x > y
end

# INITIAL-LABEL: @"func_Object#13do_gt_untyped"
# INITIAL-NOT: call i64 @sorbet_rb_int_gt
# INITIAL: call i64 @sorbet_vm_gt
# INITIAL-NOT: call i64 @sorbet_rb_int_gt
# INITIAL{LITERAL}: }

p do_gt(4, 5)
p do_gt(8, 9.0)
p do_gt(SomethingElse.new, :foo)

p do_gt_untyped(4, 5)
p do_gt_untyped(7, 3.0)
p do_gt_untyped(SomethingElse.new, [:foo])
