# frozen_string_literal: true
# typed: true
# compiled: true

def foo
  [1].each do
    begin
    rescue
    end
  end
end
