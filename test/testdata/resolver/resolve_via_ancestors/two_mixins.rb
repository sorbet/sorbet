# typed: true
module MixinA
  A = 1
end

module MixinB
  B = 1
end

class C
  include MixinA
  include MixinB

  A
  B
end
