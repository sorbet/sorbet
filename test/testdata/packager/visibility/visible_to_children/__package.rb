# frozen_string_literal: true
# typed: strict
# enable-packager: true
# allow-relaxed-packager-checks-for: SkipCheck::For

class VisibleToChildren < PackageSpec
  visible_to VisibleToChildren::*
end
