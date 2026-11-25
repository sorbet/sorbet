# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Importers::DisallowedTest < PackageSpec
  # always allowed---no visibility usages
  import Exporters::NoAnnotations

  # not allowed---this package is NOT referenced using `visible_to`
  import Exporters::ExplicitVisibleTo
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: includes explicit visibility modifiers

  # not allowed---this package is not a `test!` package
  import Exporters::TestVisibleTo
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: includes explicit visibility modifiers

  # not allowed---this package is not a `test!` package
  import Exporters::OnlyTestVisibleTo
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: includes explicit visibility modifiers
end
