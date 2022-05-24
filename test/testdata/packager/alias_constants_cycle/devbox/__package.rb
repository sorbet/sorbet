# typed: strict
# enable-packager: true

class Opus::Devbox < PackageSpec
  import Opus::Devbox::Srv

  export Opus::Devbox::Constants
end
