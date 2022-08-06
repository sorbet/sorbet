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

# INITIAL-LABEL: define internal i64 @"func_<root>.13<static-init>
# INITIAL: [[VAR:%[a-zA-Z0-9_]+]] = load i8*, i8** @addr_str_hello{{.*}}
# INITIAL: call void @sorbet_defineMethod({{.*}}[[VAR]]
# INITIAL{LITERAL}: }
