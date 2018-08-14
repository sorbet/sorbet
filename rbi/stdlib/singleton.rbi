# typed: strict

module Singleton
  module SingletonClassMethods
    Sorbet.sig.returns(T.self_type)
    def instance; end

    private

    Sorbet.sig.void
    def new; end
  end
  mixes_in_class_methods(SingletonClassMethods)
end
