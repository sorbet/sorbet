# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

extend T::Sig

sig {params(str: String, i: T.untyped).returns(T.nilable(String))}
def string_brackets(str, i)
  str[i]
end

# INITIAL-LABEL: {{^}}define{{.*}}@"func_Object#15string_brackets"
# INITIAL: call i64 @sorbet_int_rb_str_aref_m
# INITIAL{LITERAL}: }

sig {params(str: String, i: T.untyped).returns(T.nilable(String))}
def string_slice(str, i)
  str.slice(i)
end

# INITIAL-LABEL: {{^}}define{{.*}}@"func_Object#12string_slice"
# INITIAL: call i64 @sorbet_int_rb_str_aref_m
# INITIAL{LITERAL}: }

p string_brackets("123", 1)
p string_slice("123", 1)
p string_brackets("123", 0...2)
p string_slice("123", 0...2)
