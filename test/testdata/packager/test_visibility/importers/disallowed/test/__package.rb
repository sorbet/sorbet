# frozen_string_literal: true
# typed: strict

class Test::Importers::Disallowed < PackageSpec
  test!

  import Importers::Disallowed

  # always allowed---no visibility usages
  import Exporters::NoAnnotations

  # not allowed---test packages are not listed in `visible_to`
  import Exporters::ExplicitVisibleTo
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: includes explicit visibility modifiers

  # allowed---`visible_to 'tests'` grants access to test packages
  import Exporters::TestVisibleTo

  # allowed---`visible_to 'tests'` grants access to test packages
  import Exporters::OnlyTestVisibleTo
end
