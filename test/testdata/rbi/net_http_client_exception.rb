# typed: true

T.assert_type!(Net::HTTPClientException.new("message", nil), Net::HTTPClientException)
T.assert_type!(Net::HTTPServerException.new("message", nil), Net::HTTPClientException)
T.assert_type!(Net::HTTPClientException.new("message", nil), Net::ProtoServerError)

class Net::HTTPClientException
  def reopened; end
end

Net::HTTPServerException.new("message", nil).reopened
