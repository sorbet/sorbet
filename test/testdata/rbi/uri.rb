# typed: strict

extend T::Sig

sig {params(uri_string: String).returns(URI::Generic)}
def validate_http(uri_string)
  uri = URI.parse(uri_string)
  raise 'Must be an HTTP URI' unless uri.is_a?(URI::HTTP)
  uri
end

sig {returns(T::Class[T.anything])}
def uri_parser
  URI::Parser
end

sig {returns(T::Module[T.anything])}
def uri_regexp
  URI::REGEXP
end

URI::Generic.new(nil,nil,nil,nil,nil,nil,nil,nil,nil).to_str

T.assert_type!(URI::HTTP.new('http',nil,'www.example.com',nil,nil,'/foo/bar',nil,nil,nil).origin, String)
