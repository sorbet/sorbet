# typed: strict
class S3Cache
    ENCODING = T.let(::Encoding1::UTF_81, T.untyped) # error: Unable to resolve constant
end
