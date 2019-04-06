# typed: strict

module Singleton
  module SingletonClassMethods
    # Correctly modeling this return value is blocked by this issue:
    # https://github.com/stripe/sorbet/issues/62
    sig {returns(T.untyped)}
    def instance; end

    private

    sig {void}
    def new; end
  end
  mixes_in_class_methods(SingletonClassMethods)
end
