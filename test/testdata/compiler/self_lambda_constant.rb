# frozen_string_literal: true
# typed: true
# compiled: true

puts "self_top " + self.to_s
module Bug
  puts "self_before " + self.to_s
  XYZ = -> {puts "inside "+ self.to_s}
  puts "after_before " +  self.to_s
end

Bug::XYZ.call
