# typed: strict

module T::Syntax
  sig { params(other: T::Module[T.anything]).void }
  private_class_method def self.included(other); end
  sig { params(other: BasicObject).void }
  private_class_method def self.extended(other); end
end
