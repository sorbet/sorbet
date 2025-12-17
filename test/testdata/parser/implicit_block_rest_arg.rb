# typed: true
extend T::Sig

sig {params(opts: T::Hash[String, Integer]).void}
def example(opts)
  opts.each do |k|
    T.reveal_type(k) # error: `[String, Integer] (2-tuple)`
  end
  opts.each do |k,|
    #              error: Anonymous rest parameter in block args
    T.reveal_type(k) # error: `String`
  end
  opts.each do |k, _|
    T.reveal_type(k) # error: `String`
  end
end
