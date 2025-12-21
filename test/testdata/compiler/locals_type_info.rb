# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL
# run_filecheck: OPT

extend T::Sig

sig {params(queue: T::Array[T.untyped], obj: T::Array[T.untyped]).void}
def f(queue, obj)
  obj.each do |x|
    queue << x
  end
end

q = []
f(q, [1, 2, 3])
f(q, [:z, :y, :x, :w])
p q

# Check that:
#
# * We read the local
# * We applied an appropriate llvm.assume to it
# * We did a normal type test + branch to intrinsic/VM implementations
#
# The last is not strictly necessary, but it helps ensure that the VM-based codepath
# is there so that if it disappeared in some way, we have a warning besides just the
# OPT-NOT test below succeeding.

# INITIAL-LABEL: define internal i64 @"func_Object#1f$block_1"
# INITIAL: [[QUEUE:%[0-9]+]] = call i64 @sorbet_readEscapedVar{{.*}}
# INITIAL-NEXT: [[ASSUMPTION:%[0-9]+]] = call i1 @sorbet_i_isa_Array(i64 [[QUEUE]]){{.*}}
# INITIAL-NEXT: call void @llvm.assume(i1 [[ASSUMPTION]]){{.*}}
# INITIAL: sendContinuation:
# INITIAL-NEXT: [[TEST:%[0-9]+]] = call i1 @sorbet_i_isa_Array(i64 [[QUEUE]]){{.*}}
# INITIAL-NEXT: [[COND:%[0-9]+]] = call i1 @llvm.expect.i1(i1 [[TEST]], i1 true){{.*}}
# INITIAL-NEXT: br i1 [[COND]], label %"fastSymCallIntrinsic_Array_<<", label %"alternativeCallIntrinsic_Array_<<"{{.*}}
# INITIAL: "alternativeCallIntrinsic_Array_<<":
# INITIAL: call i64{{.*}}@sorbet_i_send
# INITIAL: "fastSymCallIntrinsic_Array_<<":
# INITIAL{LITERAL}: }

# We should have optimized out the alternative VM-based call path based on type assumptions.
# OPT-LABEL: define internal i64 @"func_Object#1f$block_1"
# OPT-NOT: call i64 @sorbet_callFuncWithCache
# OPT{LITERAL}: }
