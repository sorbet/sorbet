# typed: strict
# disable-fast-path: true

class AbstractRPCMethod
  extend T::Generic

  RPCInput = type_member
end

class TextDocumentHoverMethod < AbstractRPCMethod
  RPCInput = type_template # error: `RPCInput` must be declared as a type_member (not a type_template) to match the parent
end
