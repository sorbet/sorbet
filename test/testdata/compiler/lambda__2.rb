# frozen_string_literal: true
# typed: true
# compiled: true

test = 'outer-most static-init scope'
10.times do
  puts test
end

module Main

  message = 'hello,'

  # Test that the environment that's moved to the heap includes the `message`
  # local, as well as handles the `name` argument
  Foo = lambda do |name|
    puts "#{message} #{name}"
  end
end

module OtherMain
  value = 10

  Bar = lambda do
    value.times do
      puts "Other foo!"
    end
  end
end
