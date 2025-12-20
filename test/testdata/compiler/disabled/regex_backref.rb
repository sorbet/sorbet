# frozen_string_literal: true
# typed: true
# compiled: true

PATTERN = /(\S+):(\d+)/

PATTERN.match('this is foo.rb:123 in a sentence') do
    p $~

    p $&

    p $`

    p $'

    p $1
    p $2

    p $+
end
