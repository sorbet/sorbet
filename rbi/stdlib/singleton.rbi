# typed: strict

module Singleton
  module SingletonClassMethods
    sig.returns(T.self_type)
    def instance; end

    private

    sig.void
    def new; end
  end
  mixes_in_class_methods(SingletonClassMethods)
end
