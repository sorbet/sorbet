# frozen_string_literal: true
# typed: strict
# compiled: true
i = 0
while i<300000
  i += 1
  begin
    raise
  rescue
  end
end
