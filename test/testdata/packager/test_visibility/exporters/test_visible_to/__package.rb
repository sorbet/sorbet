# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Exporters::TestVisibleTo < PackageSpec
  visible_to Importers::Allowed
  visible_to Importers::AllowedTest
  visible_to 'tests'
end
