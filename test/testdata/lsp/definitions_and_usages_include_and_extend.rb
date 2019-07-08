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
  extend StaticHelpers
       # ^ usage: StaticHelpers
end
