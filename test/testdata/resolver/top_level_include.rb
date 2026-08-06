# typed: true

# Reports two errors because one of them comes from the keep_for_ide call.
include A # error: Unable to resolve
      # ^ error: Unable to resolve
