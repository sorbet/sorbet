# typed: true

module IsolatedForms # error: `IsolatedForms` was previously defined as a `class`
  class FromOtherFile; end
end

IsolatedForms::FromOtherFile
IsolatedForms::Helper
