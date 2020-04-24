# typed: false

T.lazy_resolve('::Integer') # error: in `# typed: true` or higher
T.lazy_resolve('::DoesntExist') # error: in `# typed: true` or higher
