# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

module Foo
  def self.bar
    "bar"
  end
end
puts Foo.bar

# INITIAL-LABEL: define internal i64 @"func_Foo.13<static-init>
# INITIAL: call void @sorbet_defineMethodSingleton({{.*@str_bar}}
# INITIAL{LITERAL}: }
