# typed: strict

class Regexp::FromExistingPrefix # error: File belongs to package `Producer` but defines a constant that does not match this namespace
end

class MissingPrefix::FromSyntheticPrefix # error: File belongs to package `Producer` but defines a constant that does not match this namespace
end
