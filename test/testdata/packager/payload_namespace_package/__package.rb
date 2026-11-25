# typed: strict
# enable-packager: true

# Only the root-level name is checked. It suffices for packages nested under a
# payload constant, even when the nested name itself is not defined.
class Regexp::Nested < PackageSpec # error: Package `Regexp::Nested` cannot be defined because `Regexp` is defined in Sorbet's payload
end
