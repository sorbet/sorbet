# frozen_string_literal: true
# typed: true
# compiled: true

def boo(&blk)
  begin
    p "in begin"
    yield
  ensure
    p "in ensure"
  end
end




boo do
  puts "boohey"
end

