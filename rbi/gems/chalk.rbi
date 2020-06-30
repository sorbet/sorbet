# The minimal portion of our Chalk::ODM support
# typed: __STDLIB_INTERNAL

class Chalk::ODM::Document
  include T::Props
  include T::Props::Serializable
  include T::Props::WeakConstructor
  extend T::Helpers
  abstract!
end

module Chalk::ODM::DocumentDecoratorHelper
  def self.soft_freeze_logic(instance, prop)
  end
end

class Opus::DB::Model::Mixins::Encryptable::EncryptedValue < Chalk::ODM::Document
  sig {params(options: T::Hash[T.untyped, T.untyped]).returns(Opus::DB::Model::Mixins::Encryptable::EncryptedValue)}
  def initialize(options)
  end
end
