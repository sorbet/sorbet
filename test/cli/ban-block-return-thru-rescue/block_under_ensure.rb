# frozen_string_literal: true
# typed: true
# compiled: true

def g
  yield
end

def f
  begin
    raise "boom"
  rescue
    puts "rescue"
  ensure
    g { return 99 }
  end
end
