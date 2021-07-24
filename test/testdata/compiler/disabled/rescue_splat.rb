# frozen_string_literal: true
# typed: true
# compiled: true

def rescue_ok?(cases, &blk)
  begin
    blk.call
  rescue *cases
    :ok
  else
    :ko
  end
end

cases = [ArgumentError, RegexpError]
p rescue_ok?(cases) do
  Regexp.new("?")
end
p rescue_ok?(cases) do
  1.to_a
end
p rescue_ok?(cases) do
  raise "Uncaught"
end
p rescue_ok?(cases) do
  [1, 2, 3].first(4, 5)
end
