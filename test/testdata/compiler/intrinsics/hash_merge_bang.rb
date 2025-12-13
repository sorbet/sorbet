# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

def no_block
  h1 = {x: "foo", q: "bar", z: 33}

  p (h1.merge!({x: "bar", z: 33, j: 96}))

  p h1
end

# INITIAL-LABEL: define internal i64 @"func_Object#8no_block"(
# INITIAL: call i64 @sorbet_rb_hash_update(
# INITIAL{LITERAL}: }

no_block

def with_block
  h1 = {x: "foo", q: "bar", z: 33}

  p (h1.merge!({x: "bar", z: 33, j: 96}) do |key, oldval, newval|
       "key: #{key}, oldval: #{oldval}, newval: #{oldval}"
     end)

  p h1
end

# INITIAL-LABEL: define internal i64 @"func_Object#10with_block"(
# INITIAL: call i64 @sorbet_callIntrinsicInlineBlock_noBreak(i64 (i64)* @forward_sorbet_rb_hash_update_withBlock
# INITIAL{LITERAL}: }

with_block

def with_block_raise
  h1 = {x: "foo", q: "bar", z: 33}

  begin
    h1.merge!({x: "bar", z: 33, j: 96}) do |key, oldval, newval|
      if key == :z
        raise "boom"
      end
      newval
    end
  rescue => e
    puts e
  end

  p h1
end

# INITIAL-LABEL: define internal i64 @"func_Object#16with_block_raise"(
# INITIAL: call i64 @sorbet_callIntrinsicInlineBlock_noBreak(i64 (i64)* @forward_sorbet_rb_hash_update_withBlock
# INITIAL{LITERAL}: }

with_block_raise