# typed: false

{ k1: "v1" }

{ :k2 => "v2" }

{ "k3" => "v3" }

def has_named_kwargs(**kwargs)
  { before: "value before", **kwargs, after: "value after" }
end

def has_anonymous_kwargs(**)
  { before: "value before", **, after: "value after" }
end

v4 = "v4"
k4 = { v4: }

def v5; end

v5(k5:)
