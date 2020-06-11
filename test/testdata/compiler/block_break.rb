# frozen_string_literal: true
# typed: true
# compiled: true


broken = T.let(false, T::Boolean)

loop do

  if broken
    puts "block break is broken"
    Kernel.exit
  end

  broken = true
  break
end

puts "block break is working fine"

puts ([1].map {|x| break x})
