# frozen_string_literal: true
# compiled: true
# typed: true

x = "hello, world"

p x.start_with?()

p x.start_with?("he")

p x.start_with?("no")

p x.start_with?("no", "he")

p x.start_with?(/hel+o/)

p x.start_with?(/no/, /hel+o/)

p x.start_with?("no", /hel+o/)
