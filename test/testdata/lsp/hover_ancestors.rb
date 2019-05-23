# typed: true
class Super
end
module Mod
end
class A < Super
        # ^ hover: T.class_of(Super)
  include Mod
        # ^ hover: T.class_of(Mod)
end
