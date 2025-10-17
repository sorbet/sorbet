# typed: false

[1,2,3].map do |x|
  _1 # error: numbered parameters are not allowed when an ordinary parameter is defined
end

_1
