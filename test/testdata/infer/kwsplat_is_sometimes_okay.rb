# typed: true
extend T::Sig

def takes_required_kwargs(x:, y: nil)
end

def takes_untyped_kwargs(x: nil, y: nil)
end

sig {params(x: Integer, y: String).void}
def takes_non_nil_kwargs(x: 0, y: '')
end

sig {params(x: T.nilable(Integer), y: T.nilable(String)).void}
def takes_nilable_kwargs(x: 0, y: '')
end

sig {params(opts: T::Hash[Symbol, Integer], string_opts: T::Hash[String, Integer]).void}
def example1(opts, string_opts)
  takes_required_kwargs(**opts)
  takes_untyped_kwargs(**string_opts)
  takes_untyped_kwargs(**opts)
  takes_non_nil_kwargs(**opts)
  takes_nilable_kwargs(**opts)
end
