# frozen_string_literal: true
# typed: true
# compiled: true

def rescue_ok?(cases, &blk)
  begin
    blk.call
  rescue *cases
    :rescued_from_splat
  rescue
    :rescued_from_catch_all
  else
    :no_exception
  end
end

cases = [ArgumentError, TypeError]
p (rescue_ok?(cases) do
     T.unsafe(3.0) + "aha"
   end)
p (rescue_ok?(cases) do
     T.unsafe(1).to_a
   end)
p (rescue_ok?(cases) do
     raise "Uncaught"
   end)
p (rescue_ok?(cases) do
     T.unsafe([1, 2, 3]).first(4, 5)
   end)
p (rescue_ok?(cases) do
     2+2
   end)
