# frozen_string_literal: true
# typed: true
# compiled: true

def justyield
  yield
end

def f
  puts (justyield do
          begin
            return 444
          rescue
            nil
          end
        end)
  555
end

puts "Here is f: #{f}"
