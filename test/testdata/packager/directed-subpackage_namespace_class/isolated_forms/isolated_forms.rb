# typed: true

module IsolatedForms::Before
end

class IsolatedForms # error: `IsolatedForms` cannot be a `class`, because packages are nested under `IsolatedForms`
  class Helper; end
  class FromHelper < Helper; end # error: Unable to resolve constant `Helper`
end

class ::IsolatedForms::RootScoped
end

IsolatedForms::Y = 1

IsolatedForms::Before # error: Unable to resolve constant `Before`
IsolatedForms::Helper # error: Unable to resolve constant `Helper`
::IsolatedForms::Helper # error: Unable to resolve constant `Helper`
IsolatedForms::RootScoped # error: Unable to resolve constant `RootScoped`
IsolatedForms::Y # error: Unable to resolve constant `Y`
