# typed: true

module A
  YYY = 1

  p YYY
  #    ^ completion: YYY

  p Integer
  #        ^ completion: Integer
end

p A::YYY
#       ^ completion: YYY

p Integer
#        ^ completion: Integer
