# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Exporters::OnlyTestVisibleTo < PackageSpec
  visible_to 'tests'
end
