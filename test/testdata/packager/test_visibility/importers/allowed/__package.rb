# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Importers::Allowed < PackageSpec
  # always allowed---no visibility usages
  import Exporters::NoAnnotations

  # allowed---this package is explicitly referenced in `visible_to`
  import Exporters::ExplicitVisibleTo

  # allowed---this package is explicitly referenced in `visible_to`
  import Exporters::TestVisibleTo

  # not allowed---this package is NOT referenced using `visible_to`
  import Exporters::OnlyTestVisibleTo
       # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: includes explicit visibility modifiers
end
