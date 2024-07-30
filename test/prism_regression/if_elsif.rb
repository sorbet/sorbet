# typed: strict

if true
  "one"
elsif false
#     ^^^^^ error: This code is unreachable
  "two"
else
  "three"
end
