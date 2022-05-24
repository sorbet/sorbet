# typed: strict
# enable-packager: true

class Opus::Devbox::Srv < PackageSpec
  import Opus::Devbox

  export Opus::Devbox::Srv::Routes::UserFacing
end
