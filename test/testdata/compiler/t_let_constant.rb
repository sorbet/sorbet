# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

def f
  hash = T.let({a: 1, b: 1, c: 2, d: 3, e: 5, f: 8, g: 13}, T::Hash[Symbol, Integer])
end

p f

def g
  array = T.let([1, 2, 3, 4], T::Array[Integer])
end

p g

# INITIAL-LABEL: define i64 @"func_Object#1f"
# INITIAL: [[DUP:%[a-zA-Z]+]] = call i64 @sorbet_globalConstDupHash{{.*}}
# INITIAL-NEXT: [[COND:%[0-9]+]] = call i1 @sorbet_i_isa_Hash(i64 [[DUP]]{{.*}}
# INITIAL-NEXT: call void @llvm.assume(i1 [[COND]]){{.*}}
# INITIAL-NEXT: call i1 @sorbet_i_typeTested(i64 [[DUP]]{{.*}}
# INITIAL-NEXT: br i1 true, label %typeTestSuccess, label %typeTestFail{{.*}}
# INITIAL{LITERAL}: }

# INITIAL-LABEL: define i64 @"func_Object#1g"
# INITIAL: [[DUP:%[a-zA-Z]+]] = call i64 @sorbet_buildArrayIntrinsic{{.*}}
# INITIAL-NEXT: [[COND:%[0-9]+]] = call i1 @sorbet_i_isa_Array(i64 [[DUP]]{{.*}}
# INITIAL-NEXT: call void @llvm.assume(i1 [[COND]]){{.*}}
# INITIAL-NEXT: call i1 @sorbet_i_typeTested(i64 [[DUP]]{{.*}}
# INITIAL-NEXT: br i1 true, label %typeTestSuccess, label %typeTestFail{{.*}}
# INITIAL{LITERAL}: }
