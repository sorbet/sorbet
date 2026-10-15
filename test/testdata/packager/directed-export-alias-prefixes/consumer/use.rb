# typed: true

module Consumer
  Producer::Final
  Producer::FinalTarget::Nested # error: resolves but is not exported
  Producer::ViaTarget::Nested
  Producer::ViaTarget::Value
end
