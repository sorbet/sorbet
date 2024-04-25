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
    p "running ensure"
    p "current exception #{$!}"
  end
end

v = wrap do |value|
  if value == :in_raise
    p "inside block #{$!}"
    break :early_return
  end
  :late_return
end

p "exception outside #{$!.inspect}"
p v
