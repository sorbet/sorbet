# typed: strict
# enable-packager: true

# `Child/__package.rb` sorts before this file, so the packager processes the
# subpackage before its parent.
class Root < PackageSpec
end
