def wrong_order(first="1", second)
  first + " " + second
end

def only_defaults(first="1")
  first
end

def fall_through(first="1", second="2")
  first + " " + second
end

# puts wrong_order("2")
puts wrong_order("2", "3")

puts only_defaults
puts only_defaults("2")

puts fall_through
