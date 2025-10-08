# typed: false

[1,2,3].map do |x|
  _1 # error: can't use anonymous parameters when ordinary parameters are defined
end

_1
