# typed: strict
# enable-packager: true

class Opus::Devbox::AssignmentClient
  p(Opus::Devbox::Srv::Routes::UserFacing::Helpers::MAX_NUM_ADDITIONAL_DEVBOXES) # error: `Opus::Devbox::Srv::Routes::Helpers::MAX_NUM_ADDITIONAL_DEVBOXES` resolves but is not exported from `Opus::Devbox::Srv`
end
