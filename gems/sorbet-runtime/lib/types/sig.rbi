# typed: strict

module T::Sig
  module Private
    module MethodAdded
      sig { params(method_name: Symbol).void }
      private def method_added(method_name); end
    end
    module SingletonMethodAdded
      sig { params(method_name: Symbol).void }
      private def singleton_method_added(method_name); end
    end

    TOP_SELF = T.let(self, Object)
  end

  sig { params(other: T::Module[T.anything]).void }
  private_class_method def self.included(other); end

  sig { params(other: BasicObject).void }
  private_class_method def self.extended(other); end
end
