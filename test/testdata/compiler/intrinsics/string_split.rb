# frozen_string_literal: true
# compiled: true
# typed: true
# run_filecheck: OPT

module M
  extend T::Sig

  # OPT-LABEL: define internal i64 @func_M.10test_split(i32
  sig {params(str: String).void}
  def self.test_split(str)
    # OPT: sorbet_rb_str_split_m
    p str.split(' ')

    # OPT: sorbet_rb_str_split_m
    p str.split(/\s/)

    # The current `String#split` intrinsic doesn't support blocks, so this case
    # should fall back on vm dispatch
    #
    # OPT-NOT: sorbet_rb_str_split_m
    p(str.split(/l/) {|x| puts "sub: #{x}"})
  end
  # OPT{LITERAL}: }
end

M.test_split("hello, welcome to compiled ruby")
