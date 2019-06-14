# frozen_string_literal: true
# typed: false

module T::Props
  module CustomType
    include Kernel # for `is_a?`

    # Returns true if the given Ruby value can be assigned to a T::Props field
    # of this type.
    #
    # @param [Object] _value
    # @return T::Boolean
    def instance?(_value)
      raise NotImplementedError.new('Must override in included class')
    end

    # Alias for consistent interface with T::Types::Base
    def valid?(value)
      instance?(value)
    end

    # Given an instance of this type, serialize that into a scalar type
    # supported by T::Props.
    #
    # @param [Object] _instance
    # @return An instance of one of T::Configuration.scalar_types
    def serialize(_instance)
      raise NotImplementedError.new('Must override in included class')
    end

    # Given the serialized form of your type, this returns an instance
    # of that custom type representing that value.
    #
    # @param _mongo_scalar One of T::Configuration.scalar_types
    # @return Object
    def deserialize(_mongo_scalar)
      raise NotImplementedError.new('Must override in included class')
    end

    def self.included(_base)
      super

      raise 'Please use "extend", not "include" to attach this module'
    end

    def self.scalar_type?(val)
      # We don't need to check for val's included modules in
      # T::Configuration.scalar_types, because T::Configuration.scalar_types
      # are all classes.
      klass = val.class
      until klass.nil?
        return true if T::Configuration.scalar_types.include?(klass.to_s)
        klass = klass.superclass
      end
      false
    end

    # We allow custom types to serialize to Arrays, so that we can
    # implement set-like fields that store a unique-array, but forbid
    # hashes; Custom hash types should be implemented via an emebdded
    # T::Struct (or a subclass like Chalk::ODM::Document) or via T.
    def self.valid_serialization?(val, type=nil)
      if type&.name == 'Chalk::ODM::BsonTypes::BsonObject'
        # Special case we allow for backwards compatibility with props formerly
        # typed as "Object" or "Hash", which contain arbitrarily-nested BSON
        # data (e.g. parsed API request bodies). In general, we aren't pushing
        # to convert these to Chalk::ODM::BsonTypes - we'd rather delurk them -
        # but this lets us convert events with these types to Proto.
        return true
      end

      case val
      when Array
        val.each do |v|
          return false unless scalar_type?(v)
        end

        true
      else
        scalar_type?(val)
      end
    end
  end
end
