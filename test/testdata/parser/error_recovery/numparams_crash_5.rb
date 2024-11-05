# typed: false

[1,2,3].map do |x|
  _1 # error: can't use numbered params
end

_1
