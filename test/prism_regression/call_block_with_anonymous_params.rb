# typed: false
# disable-parser-comparison: true

foo do |*, **, &|
  "block with anonymous rest, kwrest and block params"
end
