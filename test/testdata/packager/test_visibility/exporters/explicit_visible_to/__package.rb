# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Exporters::ExplicitVisibleTo < PackageSpec
  visible_to Importers::Allowed
  visible_to Importers::AllowedTest
end
