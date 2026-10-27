# typed: strict
# stratum: 0

# `Parent` imports this package, so in package-directed mode `Parent`'s
# definitions are not entered yet when this file is resolved. `Parent` is not
# imported here, and every constant below can only be owned by `Parent`.

module Parent::Utils::Child
  class Client
    Parent::TopLevel
  # ^^^^^^ error: `Parent` is not imported

    Parent::Middleware::RequestContext
  # ^^^^^^ error: `Parent` is not imported

    Parent::Utils::Auth
  # ^^^^^^ error: `Parent` is not imported

    Parent::DoesNotExist
  # ^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `DoesNotExist`

    Parent::Utils::DoesNotExist
  # ^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `DoesNotExist`
  end
end
