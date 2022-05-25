# typed: strict
module Constants
  MAX_NUM_ADDITIONAL_DEVBOXES = 200
end
module Routes::Helpers
  MAX_NUM_ADDITIONAL_DEVBOXES = Constants::MAX_NUM_ADDITIONAL_DEVBOXES
end
module Routes::WillCollapseOut
  Helpers = Routes::Helpers
end
Routes::WillCollapseOut::Helpers::MAX_NUM_ADDITIONAL_DEVBOXES
