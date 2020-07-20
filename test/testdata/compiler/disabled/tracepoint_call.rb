# frozen_string_literal: true
# typed: true
# compiled: true

def foo
end

count = 0
TracePoint.trace(:call) do |t|
  if t.method_id == :foo
    count += 1
  end
end

p count
foo
p count
foo
p count
