# typed: true
module MixinA
  A = 1
end

module MixinB
  include MixinA

  A
end

class Test
  include MixinB

  A
end
