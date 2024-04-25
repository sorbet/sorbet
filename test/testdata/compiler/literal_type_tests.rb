# frozen_string_literal: true
# typed: true
# compiled: true

def literal_symbol(obj)
  case obj
  when :matching
    true
  else
    false
  end
end

def literal_string(obj)
  case obj
  when "matching"
    true
  else
    false
  end
end

def literal_double(obj)
  3.14 <= obj
end

def literal_integer(obj)
  15 < obj
end

# INITIAL-NEXT: [[ID:%[_a-zA-Z0-9]+]] = load i64, i64* [[ID_ADDR]]{{.*}}
# INITIAL-NEXT: [[SYM:%[_a-zA-Z]+]] = call i64 @rb_id2sym(i64 [[ID]]{{.*}}
# INITIAL-NEXT: [[COND:%[0-9]+]] = call i1 @sorbet_i_isa_Symbol(i64 [[SYM]]{{.*}}
# INITIAL-NEXT: call void @llvm.assume(i1 [[COND]]){{.*}}
# INITIAL-NEXT: br i1 true, label %"fastSymCallIntrinsic_Symbol_===", label %"alternativeCallIntrinsic_Symbol_==="{{.*}}

# INITIAL-NEXT: [[STRING:%[_a-zA-Z]+]] = load i64, i64* [[STRING_TMP]]{{.*}}
# INITIAL-NEXT: [[COND:%[0-9]+]] = call i1 @sorbet_i_isa_String(i64 [[STRING]]{{.*}}
# INITIAL-NEXT: call void @llvm.assume(i1 [[COND]]){{.*}}
# INITIAL-NEXT: br i1 true, label %"fastSymCallIntrinsic_String_===", label %"alternativeCallIntrinsic_String_==="{{.*}}

# INITIAL-NEXT: [[COND:%[0-9]+]] = call i1 @sorbet_i_isa_Float(i64 [[DOUBLE]]{{.*}}
# INITIAL-NEXT: call void @llvm.assume(i1 [[COND]]){{.*}}
# INITIAL-NEXT: br i1 true, label %"fastSymCallIntrinsic_Float_<=", label %"alternativeCallIntrinsic_Float_<="{{.*}}

# INITIAL-NEXT: [[COND:%[0-9]+]] = call i1 @sorbet_i_isa_Integer(i64 [[INTEGER]]{{.*}}
# INITIAL-NEXT: call void @llvm.assume(i1 [[COND]]){{.*}}
# INITIAL-NEXT: br i1 true, label %"fastSymCallIntrinsic_Integer_<", label %"alternativeCallIntrinsic_Integer_<"{{.*}}

