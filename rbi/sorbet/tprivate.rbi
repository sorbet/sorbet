# typed: true

# These things are things that Sorbet considers unstable internal APIs that
# could (and do) change at any moment without notice.
#
# Use them at your own risk.

module T::Private::Types
end

class T::Private::Types::Void < T::Types::Base
end

class T::Private::Types::TypeAlias < T::Types::Base
  sig { params(callable: T.proc.returns(T.anything)).void }
  def initialize(callable); end

  sig {params(arg: Symbol).returns(T::Private::Types::TypeAlias)}
  def checked(arg); end

  sig { override.void }
  def build_type; end

  sig {returns(T::Types::Base)}
  def aliased_type; end

  sig {returns(T::Types::Base)}
  def effective_aliased_type; end

  sig { override.returns(String) }
  def name; end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def recursively_valid?(obj); end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end
end

module T::Private::Methods
  def self.signature_for_method(method); end
end

module T::Private::Methods::CallValidation
  def self.disable_fast_path; end
end
