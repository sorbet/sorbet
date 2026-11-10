# typed: true

module IsolatedForms::Before
end

class IsolatedForms # error: `IsolatedForms` cannot be a `class`, because packages are nested under `IsolatedForms`
  class Helper; end
  class FromHelper < Helper; end
end

class ::IsolatedForms::RootScoped
end

IsolatedForms::Y = 1

IsolatedForms::Before
IsolatedForms::Helper
::IsolatedForms::Helper
IsolatedForms::RootScoped
IsolatedForms::Y
