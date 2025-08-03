# frozen_string_literal: true
# typed: true
# enable-packager: true

# This file should also be in __UNPACKAGED__ and can reference other unpackaged code
class AnotherUnpackagedClass
  def self.use_unpackaged_class
    UnpackagedClass.hello
  end
end