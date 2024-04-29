# typed: false

def has_named_kwargs(**kwargs)
  delegate(**kwargs)
end

def has_anonymous_kwargs(**)
  delegate(**)
end
