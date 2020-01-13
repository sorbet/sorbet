# typed: true

module MixinA
  YYY = 1
end

module MixinB
  include MixinA

  YY # error: Unable to resolve
  # ^ completion: YYY
end

class Has_Result_via_B_via_A
  include MixinB

  YY # error: Unable to resolve
  # ^ completion: YYY
end
