# typed: true
def foo
  bar
end

def bar
  bt = Thread.current.backtrace.join("\n")
  bt = bt.gsub(/^.*com_stripe_sorbet_llvm\//, '')
  bt = bt.gsub(/^.*ruby_2_6_3\//, '')
  bt = bt.gsub(/^.*tmp\..*:/, ':')
  # for now line numbers are off
  bt = bt.gsub(/:[0-9]*/, ':')
  # for now (maybe?) the monkeypatching has different backtraces
  bt = bt.gsub("lib/ruby/2.6.0/rubygems/core_ext/kernel_require.rb::in `require'", '')
  bt = bt.gsub("run/tools/patch_require.rb::in `require'", '')
  bt = bt.gsub("\n\n", "\n")
  bt
end

puts foo
