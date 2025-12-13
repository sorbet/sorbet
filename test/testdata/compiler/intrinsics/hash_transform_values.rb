# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

module M

  # INITIAL-LABEL: define internal i64 @func_M.10test_block(
  # INITIAL: call i64 @sorbet_callIntrinsicInlineBlock_noBreak({{.*}}@forward_sorbet_rb_hash_transform_values_withBlock
  # INITIAL-NOT: @sorbet_i_send
  # INITIAL{LITERAL}: }
  def self.test_block()
    {a: 1, b: 2, c: 3}.transform_values do |a|
      p " #{a}"
      a + 1
    end
  end

  # INITIAL-LABEL: define internal i64 @func_M.9test_enum(
  # INITIAL: call i64 @sorbet_rb_hash_transform_values(
  # INITIAL-NOT: @sorbet_i_send
  # INITIAL{LITERAL}: }
  def self.test_enum()
    {a: 1, b: 2, c: 3}.transform_values
  end

end

p(M.test_block)
p(M.test_enum.each {|x| p "enum #{x}"})

