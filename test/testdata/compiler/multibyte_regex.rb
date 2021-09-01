# frozen_string_literal: true
# typed: true
# compiled: true

class Placeholder
  START = "\u{e000}"
  END_ = "\u{e001}"

  def self.fill_regex
    Regexp.new("#{START}[^#{END_}]+#{END_}")
  end
end

p Placeholder.fill_regex

