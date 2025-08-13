# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a, b

class NonExistentMethod < PackageSpec
  strict_dependencies 'false'
  layer 'a'
  extra 'a'
  does_not_exist 'a' # error: Method `does_not_exist` does not exist on `T.class_of(NonExistentMethod)`
end
