# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

def no_block
  h1 = {q: 99, z: 48, t: 97, p: 100}
  e = h1.select
  puts (e.each do |k, v|
          puts "#{k} => #{v}"
          v%2==0 || k==:q
        end)
end

no_block

# INITIAL-LABEL: define internal i64 @"func_Object#8no_block"(
# INITIAL: call i64 @sorbet_rb_hash_select(
# INITIAL{LITERAL}: }

def with_block
  h1 = {q: 99, z: 48, t: 97, p: 100}
  puts (h1.select do |k, v|
          puts "#{k} => #{v}"
          v%2==0 || k==:q
        end)
end

with_block

# INITIAL-LABEL: define internal i64 @"func_Object#10with_block"(
# INITIAL: call i64 @sorbet_callIntrinsicInlineBlock_noBreak(i64 (i64)* @forward_sorbet_rb_hash_select_withBlock
# INITIAL{LITERAL}: }
