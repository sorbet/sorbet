# frozen_string_literal: true
# typed: true
# compiled: true

def g
  yield
end

def f
  g do
    begin
      puts "begin"
    rescue
      puts "rescue"
    else
      return 99
    end
  end
end
