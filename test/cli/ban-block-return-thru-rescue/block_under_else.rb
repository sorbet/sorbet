# frozen_string_literal: true
# typed: true
# compiled: true

def g
  yield
end

def f
  begin
    puts "begin"
  rescue
    puts "rescue"
  else
    g { return 99 }
  end
end
