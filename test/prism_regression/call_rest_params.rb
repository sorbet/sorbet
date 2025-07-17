
# typed: false

def has_named_rest_args(*args)
  delegate(*args)
end

def has_anonymous_rest_args(*)
  delegate(*)
end
