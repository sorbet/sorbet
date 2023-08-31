# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Importers::AllowedTest < PackageSpec
  # always allowed---no visibility usages
  test_import Exporters::NoAnnotations

  # allowed---this package is explicitly referenced in `visible_to`
  test_import Exporters::ExplicitVisibleTo

  # allowed---this package is explicitly referenced in `visible_to`
  test_import Exporters::TestVisibleTo

  # allowed---not mentioned, but `visible_to 'tests'` permits this
  test_import Exporters::OnlyTestVisibleTo
end
