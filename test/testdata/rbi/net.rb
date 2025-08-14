# typed: strong

Net::HTTP.get("sub.example.com", "/some/path")
Net::HTTP.get("sub.example.com", "/some/path", 80)
Net::HTTP.get(URI("https://sub.example.com/some/path"))
Net::HTTP.get(URI("https://sub.example.com/some/path"), { "Accept" => "application/json" })

# Ruby doesn't error on this, but will completely ignore the third argument so it's erroneous to supply it
Net::HTTP.get(URI("https://sub.example.com/some/path"), { "Accept" => "application/json" }, 443) # error: Expected `String` but found `URI::Generic` for argument `uri_or_host`
                                                                                                 # error: Expected `String` but found `{String("Accept") => String("application/json")}` for argument `path_or_header`

# Invalid combinations
Net::HTTP.get("www.example.com") # error: Not enough arguments provided for method `Net::HTTP.get (overload.2)`. Expected: `2..3`, got: `1`
Net::HTTP.get("www.example.com", { "Accept" => "application/json" }) # error: Expected `URI::Generic` but found `String("www.example.com")` for argument `uri_or_host`
Net::HTTP.get(URI("https://sub.example.com"), "/some/path") # error: Expected `T.nilable(T::Hash[T.any(String, Symbol), String])` but found `String("/some/path")` for argument `path_or_header
