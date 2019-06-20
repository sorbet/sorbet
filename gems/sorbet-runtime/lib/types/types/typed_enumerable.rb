# frozen_string_literal: true
# typed: true

module T::Types
  # Note: All subclasses of Enumerable should add themselves to the
  # `case` statement below in `describe_obj` in order to get better
  # error messages.
  class TypedEnumerable < Base
    attr_reader :type

    def initialize(type)
      @type = T::Utils.coerce(type)
    end

    # @override Base
    def name
      "T::Enumerable[#{@type.name}]"
    end

    # @override Base
    def valid?(obj)
      return false unless obj.is_a?(Enumerable)
      case obj
      when Array
        begin
          it = 0
          while it < obj.count
            return false unless @type.valid?(obj[it])
            it += 1
          end
          return true
        end
      when Hash
        return false unless @type.is_a?(FixedArray)
        types = @type.types
        return false if types.count != 2
        key_type = types[0]
        value_type = types[1]
        obj.each_pair do |key, val|
          # Some objects (I'm looking at you Rack::Utils::HeaderHash) don't
          # iterate over a [key, value] array, so we can't juse use the @type.valid?(v)
          return false if !key_type.valid?(key) || !value_type.valid?(val)
        end
        return true
      when Enumerator::Lazy
        # Users don't want these walked
        return true
      when Enumerator
        obj.each do |elem|
          return false unless @type.valid?(elem)
        end
        return true
      when Range
        @type.valid?(obj.first) && @type.valid?(obj.last)
      when Set
        obj.each do |item|
          return false unless @type.valid?(item)
        end

        return true
      else
        # We don't check the enumerable since it isn't guaranteed to be
        # rewindable (e.g. STDIN) and it may be expensive to enumerate
        # (e.g. an enumerator that makes an HTTP request)"
        true
      end
    end

    # @override Base
    private def subtype_of_single?(other)
      if self.class <= other.class
        # Enumerables are covariant because they are read only
        #
        # Properly speaking, many Enumerable subtypes (e.g. Set)
        # should be invariant because they are mutable and support
        # both reading and writing. However, Sorbet treats *all*
        # Enumerable subclasses as covariant for ease of adoption.
        @type.subtype_of?(other.type)
      else
        false
      end
    end

    # @override Base
    def describe_obj(obj)
      return super unless obj.is_a?(Enumerable)
      type_from_instance(obj).name
    end

    private def type_from_instances(objs)
      return objs.class unless objs.is_a?(Enumerable)
      obtained_types = []
      begin
        objs.each do |x|
          obtained_types << type_from_instance(x)
        end
      rescue
        return T.untyped # all we can do is go with the types we have so far
      end
      if obtained_types.count > 1
        # Multiple kinds of bad types showed up, we'll suggest a union
        # type you might want.
        Union.new(obtained_types)
      elsif obtained_types.empty?
        T.noreturn
      else
        # Everything was the same bad type, lets just show that
        obtained_types.first
      end
    end

    private def type_from_instance(obj)
      if [true, false].include?(obj)
        return T::Boolean
      elsif !obj.is_a?(Enumerable)
        return obj.class
      end

      case obj
      when Array
        T::Array[type_from_instances(obj)]
      when Hash
        inferred_key = type_from_instances(obj.keys)
        inferred_val = type_from_instances(obj.values)
        T::Hash[inferred_key, inferred_val]
      when Range
        T::Range[type_from_instances([obj.first, obj.last])]
      when Enumerator
        T::Enumerator[type_from_instances(obj)]
      when Set
        T::Set[type_from_instances(obj)]
      when IO
        # Short circuit for anything IO-like (File, etc.). In these cases,
        # enumerating the object is a destructive operation and might hang.
        obj.class
      else
        self.class.new(type_from_instances(obj))
      end
    end
  end
end
