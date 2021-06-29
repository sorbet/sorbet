# typed: __STDLIB_INTERNAL

module MessagePack
  DefaultFactory = ::T.let(nil, ::T.untyped)
  VERSION = ::T.let(nil, ::T.untyped)

  sig do
    params(
      v: ::T.untyped,
      rest: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.dump(v, *rest); end

  sig do
    params(
      src: ::T.untyped,
      param: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.load(src, param=T.unsafe(nil)); end

  sig do
    params(
      v: ::T.untyped,
      rest: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.pack(v, *rest); end

  sig do
    params(
      src: ::T.untyped,
      param: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.unpack(src, param=T.unsafe(nil)); end
end

class MessagePack::Buffer
  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def <<(arg0); end

  sig {returns(::T.untyped)}
  def clear(); end

  sig {returns(::T.untyped)}
  def close(); end

  sig {returns(::T.untyped)}
  def empty?(); end

  sig {returns(::T.untyped)}
  def flush(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(*arg0); end

  sig {returns(::T.untyped)}
  def io(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def read(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def read_all(*arg0); end

  sig {returns(::T.untyped)}
  def size(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def skip(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def skip_all(arg0); end

  sig {returns(::T.untyped)}
  def to_a(); end

  sig {returns(::T.untyped)}
  def to_s(); end

  sig {returns(::T.untyped)}
  def to_str(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def write(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def write_to(arg0); end
end

module MessagePack::CoreExt
  sig do
    params(
      packer_or_io: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def to_msgpack(packer_or_io=T.unsafe(nil)); end
end

class MessagePack::ExtensionValue < Struct
  include ::MessagePack::CoreExt
  Elem = type_member(fixed: T.untyped)

  sig {returns(::T.untyped)}
  def payload(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def payload=(arg0); end

  sig {returns(::T.untyped)}
  def type(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def type=(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.[](*arg0); end

  sig {returns(::T.untyped)}
  def self.members(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.new(*arg0); end
end

class MessagePack::Factory
  sig do
    params(
      v: ::T.untyped,
      rest: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def dump(v, *rest); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(*arg0); end

  sig do
    params(
      src: ::T.untyped,
      param: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def load(src, param=T.unsafe(nil)); end

  sig do
    params(
      v: ::T.untyped,
      rest: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def pack(v, *rest); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def packer(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def register_type(*arg0); end

  sig do
    params(
      selector: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def registered_types(selector=T.unsafe(nil)); end

  sig do
    params(
      klass_or_type: ::T.untyped,
      selector: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def type_registered?(klass_or_type, selector=T.unsafe(nil)); end

  sig do
    params(
      src: ::T.untyped,
      param: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unpack(src, param=T.unsafe(nil)); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def unpacker(*arg0); end
end

class MessagePack::MalformedFormatError < MessagePack::UnpackError
end

class MessagePack::Packer
  sig {returns(::T.untyped)}
  def buffer(); end

  sig {returns(::T.untyped)}
  def clear(); end

  sig {returns(::T.untyped)}
  def compatibility_mode?(); end

  sig {returns(::T.untyped)}
  def empty?(); end

  sig {returns(::T.untyped)}
  def flush(); end

  sig {returns(::T.untyped)}
  def full_pack(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(*arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def pack(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def register_type(*arg0); end

  sig {returns(::T.untyped)}
  def registered_types(); end

  sig {returns(::T.untyped)}
  def size(); end

  sig {returns(::T.untyped)}
  def to_a(); end

  sig {returns(::T.untyped)}
  def to_s(); end

  sig {returns(::T.untyped)}
  def to_str(); end

  sig do
    params(
      klass_or_type: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def type_registered?(klass_or_type); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def write(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def write_array(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def write_array_header(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
      arg1: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def write_ext(arg0, arg1); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def write_extension(arg0); end

  sig {returns(::T.untyped)}
  def write_false(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def write_float(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def write_float32(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def write_hash(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def write_int(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def write_map_header(arg0); end

  sig {returns(::T.untyped)}
  def write_nil(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def write_string(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def write_symbol(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def write_to(arg0); end

  sig {returns(::T.untyped)}
  def write_true(); end
end

class MessagePack::StackError < MessagePack::UnpackError
end

module MessagePack::TypeError
end

class MessagePack::UnexpectedTypeError < MessagePack::UnpackError
  include ::MessagePack::TypeError
end

class MessagePack::UnknownExtTypeError < MessagePack::UnpackError
end

class MessagePack::UnpackError < StandardError
end

class MessagePack::Unpacker
  sig {returns(::T.untyped)}
  def allow_unknown_ext?(); end

  sig {returns(::T.untyped)}
  def buffer(); end

  sig {returns(::T.untyped)}
  def each(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def feed(arg0); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def feed_each(arg0); end

  sig {returns(::T.untyped)}
  def full_unpack(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(*arg0); end

  sig {returns(::T.untyped)}
  def read(); end

  sig {returns(::T.untyped)}
  def read_array_header(); end

  sig {returns(::T.untyped)}
  def read_map_header(); end

  sig do
    params(
      arg0: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def register_type(*arg0); end

  sig {returns(::T.untyped)}
  def registered_types(); end

  sig {returns(::T.untyped)}
  def reset(); end

  sig {returns(::T.untyped)}
  def skip(); end

  sig {returns(::T.untyped)}
  def skip_nil(); end

  sig {returns(::T.untyped)}
  def symbolize_keys?(); end

  sig do
    params(
      klass_or_type: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def type_registered?(klass_or_type); end

  sig {returns(::T.untyped)}
  def unpack(); end
end
