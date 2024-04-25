# frozen_string_literal: true
# typed: true
# compiled: true

def f
  begin
    puts "begin"
  ensure
    return 99
  end
end
