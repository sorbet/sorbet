# typed: strict

# These things are things that Sorbet considers unstable internal APIs that
# could (and do) change at any moment without notice.
#
# Use them at your own risk.

module T::Private
end

module T::Private::Types
end

class T::Private::Types::Void < T::Types::Base
end

module T::Private::Methods
  def self.signature_for_method(method); end
end

module T::Private::Methods::CallValidation
  def self.disable_fast_path; end
end
