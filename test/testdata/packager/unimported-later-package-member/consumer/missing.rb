# typed: true

module Consumer
  # `Provider::DoesNotExist` is not defined anywhere. Monolithic mode can see
  # all of `Provider`, and reports this as an unresolved constant. In
  # package-directed mode, `Provider` has not been loaded yet, so there is no
  # way to distinguish this from `Provider::Flag`.
  DoesNotExist = Provider::DoesNotExist # error: Unable to resolve constant `DoesNotExist`
end
