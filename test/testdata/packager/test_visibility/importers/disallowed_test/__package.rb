# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Importers::DisallowedTest < PackageSpec
  # always allowed---no visibility usages
  test_import Exporters::NoAnnotations

  # not allowed---this package is NOT referenced using `visible_to`
  test_import Exporters::ExplicitVisibleTo
            # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: includes explicit visibility modifiers

  # allowed---not mentioned, but `visible_to 'tests'` permits this
  test_import Exporters::TestVisibleTo

  # allowed---not mentioned, but `visible_to 'tests'` permits this
  test_import Exporters::OnlyTestVisibleTo
end
