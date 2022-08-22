# this sigil is ignored in autogen mode
# # typed: strong
class Foo < Bar
end
class Bar < Foo
end

class Opus::Autogen::Proto::ProtobufGeneratedClass1
end

module Opus::Autogen::Proto
  ProtobufGeneratedClass1 = T.unsafe(Class).new
end

module Opus::Autogen::Proto
  ProtobufGeneratedClass2 = T.unsafe(Class).new
end

class Opus::Autogen::Proto::ProtobufGeneratedClass2
end
