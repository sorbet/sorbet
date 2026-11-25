# typed: true

module IsolatedForms
  class FromOtherFile; end
end

IsolatedForms::FromOtherFile
IsolatedForms::Helper # error: Unable to resolve constant `Helper`
