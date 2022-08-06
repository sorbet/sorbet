# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

puts ["ha","ho","hey"].join("---")
puts [1,2,"hey"].join(",")
puts [1,2,3].join

# INITIAL-LABEL: define internal i64 @"func_<root>.13<static-init>
# INITIAL: call i64 @sorbet_int_rb_ary_join_m(
# INITIAL: call i64 @sorbet_int_rb_ary_join_m(
# INITIAL: call i64 @sorbet_int_rb_ary_join_m(
# INITIAL{LITERAL}: }
