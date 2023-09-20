# typed: true

class Module; include T::Sig; end

class MyTEnum
  extend T::Generic
  abstract!

  LinkedGenericTypePair = type_member

  sig {params(serialized_val: LinkedGenericTypePair).void}
  def initialize(serialized_val)
    @serialized_val = serialized_val
  end

  sig {returns(LinkedGenericTypePair)}
  def serialize; @serialized_val; end

  class << self
    extend T::Generic
    LinkedGenericTypePair = type_member

    sig {params(serialized_val: LinkedGenericTypePair).returns(T.nilable(T.attached_class))}
    def try_deserialize(serialized_val)
      raise "Not initialized" if @mapping.nil?
      deserialized = @mapping[serialized_val]
      T.reveal_type(deserialized)
      deserialized
    end

    sig {params(blk: T.proc.void).void}
    def enums(&blk)
      @values = T.let(nil, T.nilable(T::Array[T.attached_class]))
      yield
      @mapping = T.let(nil, T.nilable(T::Hash[LinkedGenericTypePair, T.attached_class]))
      @mapping = {}

      # ... populate mapping ...
    end
  end
end

sig do
  params(
    klass: T.class_of(MyTEnum)[MyTEnum[String], String],
    serialized_val: String,
  )
    .returns(MyTEnum[String])
end
def example1(klass, serialized_val)
  instance = klass.try_deserialize(serialized_val)
  T.reveal_type(instance)
  result = T.must(instance)
  result
end

sig do
  params(
    obj: MyTEnum[String],
    serialized_val: String,
  )
    .returns(T.class_of(MyTEnum)[MyTEnum[String], String])
end
def example2(obj, serialized_val)
  klass = obj.class
  T.reveal_type(klass)
  instance = klass.try_deserialize(serialized_val)
  T.reveal_type(instance)
  result = T.must(instance)
  T.reveal_type(result.class)
  result.class
end

