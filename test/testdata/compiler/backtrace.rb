# typed: true
def foo
  bar
end

def bar
  bt = Thread.current.backtrace.join("\n")
  bt = bt.gsub(/^.*com_stripe_sorbet_llvm\//, '')
  bt = bt.gsub(/^.*tmp\..*:/, ':')
  # for now line numbers are off
  bt = bt.gsub(/:[0-9]*/, ':')
  bt
end

puts foo
