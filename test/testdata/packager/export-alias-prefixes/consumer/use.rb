# typed: true

module Consumer
  Producer::Final
  Producer::FinalTarget::Nested # error: resolves but is not exported
  Producer::ViaTarget::Nested # error: resolves but is not exported
  Producer::ViaTarget::Value # error: resolves but is not exported
  Producer::AppTarget::Nested # error: resolves but is not exported
end
