# frozen_string_literal: true
# typed: true
# compiled: true
def foo
  bar
end

def bar
  bt = Thread.current.backtrace.join("\n")
  bt = bt.gsub(/^.*com_stripe_sorbet_llvm\//, '')
  bt = bt.gsub(/^.*sorbet_ruby\//, '')
  bt = bt.gsub(/^.*tmp\..*:/, ':')

  # for now (maybe?) the monkeypatching has different backtraces
  bt = bt.gsub("lib/ruby/2.6.0/rubygems/core_ext/kernel_require.rb:54:in `require'", '')
  bt = bt.gsub("run/tools/patch_require.rb:29:in `require'", '')
  bt = bt.gsub(%r{.*run/tools/patch_require.rb:37:in `require'}, '')
  bt = bt.gsub("\n\n", "\n")

  bt
end

puts foo
