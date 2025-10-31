# typed: false

{ k1: "v1" } # Symbol location excludes the `:`

{ :k2 => "v2" }

{ "k3" => "v3" }

def has_named_kwargs(**kwargs)
  { before: "value before", **kwargs, after: "value after" }
end

def has_anonymous_kwargs(**)
  { before: "value before", **, after: "value after" }
end

v4 = "v4"

# Hash with implicit lvar value
k4 = { v4: } # Symbol location excludes the `:`

def v5; end

# Hash with implicit method value
v5 = { v5: } # Symbol location excludes the `:`
