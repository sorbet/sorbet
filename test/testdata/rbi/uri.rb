# typed: strict

extend T::Sig

sig {params(uri_string: String).returns(URI::Generic)}
def validate_http(uri_string)
  uri = URI.parse(uri_string)
  raise 'Must be an HTTP URI' unless uri.is_a?(URI::HTTP)
  uri
end

sig {returns(Class)}
def uri_parser
  URI::Parser
end

sig {returns(Module)}
def uri_regexp
  URI::REGEXP
end
