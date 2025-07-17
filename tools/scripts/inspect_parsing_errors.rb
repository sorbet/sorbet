#!/usr/bin/env ruby

require "prism"

file_path = ARGV[0]

parsed = Prism.parse_file(file_path)

pp parsed.value

if parsed.errors.empty?
  puts "No parse errors found"
  exit
end

puts
puts "======================= Parse errors ======================="
puts

parsed.errors.each do |error|
  pp error
end
