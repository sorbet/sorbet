# frozen_string_literal: true
# typed: true
# compiled: true

def test
  begin
    return true
  ensure
    puts "ensure"
  end
end
