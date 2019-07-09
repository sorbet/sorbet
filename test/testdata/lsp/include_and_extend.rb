# typed: true

module InstanceHelpers
     # ^ def: InstanceHelpers
end

module StaticHelpers
    #  ^ def: StaticHelpers
end

class Foo
  include InstanceHelpers
        # ^ usage: InstanceHelpers
        # ^ hover: T.class_of(InstanceHelpers)
  extend StaticHelpers
       # ^ usage: StaticHelpers
       # ^ hover: T.class_of(StaticHelpers)
end
