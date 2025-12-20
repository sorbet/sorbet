# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL
# run_filecheck: OPT

extend T::Sig

sig {params(str: String, obj: T.untyped).returns(T::Boolean)}
def streq(str, obj)
  str == obj
end

# INITIAL-LABEL: "func_Object#5streq"
# INITIAL: call i64 @sorbet_int_rb_str_equal
# INITIAL{LITERAL}: }

# OPT-LABEL: "func_Object#5streq"
# OPT-NOT: call i64 @sorbet_int_rb_str_equal
# OPT: call i64 @rb_str_equal
# OPT-NOT: call i64 @sorbet_int_rb_str_equal
# OPT{LITERAL}: }

p streq("str", 1)
p streq("str", "str")
p streq("str", nil)
