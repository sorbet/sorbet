# frozen_string_literal: true
# typed: strict
# compiled: true
i = 0
while i < 10_000_000
  begin
    raise
  rescue
  end
  i += 1
end

p i
