# frozen_string_literal: true
# typed: true
# compiled: true

def wrap(&blk)
  begin
    p "starting begin"
    raise 'exception'
  rescue
    p "running raise #{$!}"
    yield :in_raise
  ensure
    p "running ensure #{$!}"
    raise 'from ensure'
  end
end

v = :unmodified

begin
  v = wrap do |value|
    if value == :in_raise
      p "inside block #{$!}"
      break :early_return
    end
    :late_return
  end
  p "got after v"
rescue Exception => e
  p "rescued outer exception"
  p "have #{$!}"
  p "actually rescued #{e}"
end

p "exception outside #{$!.inspect}"
p v
