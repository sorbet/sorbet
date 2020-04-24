# typed: false

T::LazyConstants.lazy_is_a?('::Integer') # error: in `# typed: true` files
T::LazyConstants.lazy_is_a?('::DoesntExist') # error: in `# typed: true` files
