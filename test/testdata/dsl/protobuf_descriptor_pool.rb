# typed: false

# This is what the protoc output looks like
ProtocMsgClass = Google::Protobuf::DescriptorPool.generated_pool.lookup("...").msgclass # error: Unable to resolve constant `Google`
ProtocMsgClass::ProtocEnumModule = Google::Protobuf::DescriptorPool.generated_pool.lookup("...").enummodule # error: Unable to resolve constant `Google`

# pay-server post-processes the protoc output from 'Google' -> '::Google'
StripeMsgClass = ::Google::Protobuf::DescriptorPool.generated_pool.lookup("...").msgclass # error: Unable to resolve constant `Google`
StripeMsgClass::StripeEnumModule = ::Google::Protobuf::DescriptorPool.generated_pool.lookup("...").enummodule # error: Unable to resolve constant `Google`

# attempting to catch false positives
Skip1 = Protobuf::DescriptorPool.generated_pool.lookup("...").msgclass # error: Unable to resolve constant `Protobuf`
Skip2 = msgclass
Skip3 = enummodule
