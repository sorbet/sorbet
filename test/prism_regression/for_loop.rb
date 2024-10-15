# typed: false

for x in a
end

for x in a
  "one-liner"
end

for x in a
  "two"
  "lines"
end

for x in a do # Has the optional `do` keyword, but it translates the same.
end
