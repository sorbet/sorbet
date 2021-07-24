# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

def hello(name)
   i = 0
   while (i < 10)
     puts("hello " + name)
     i += 1
   end
end
hello("sorbet")

# INITIAL-LABEL: define internal i64 @"func_<root>.<static-init>
# INITIAL: call void @sorbet_defineMethod({{.*@str_hello}}
# INITIAL{LITERAL}: }
