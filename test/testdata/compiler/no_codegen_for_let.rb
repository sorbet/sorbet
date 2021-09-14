# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

def bar(x)
  y = T.let(x, T.nilable(String))
  y.to_s
end

p bar('baz')
p bar(nil)

# INITIAL-LABEL: @"func_Object#3bar"
# INITIAL-NOT: sorbet_i_getRubyClass{{.*}}str_T
# INITIAL-NOT: sorbet_callFuncWithCache{{.*}}ic_nilable
# INITIAL: {{^[}]}}
