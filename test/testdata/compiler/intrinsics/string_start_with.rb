# frozen_string_literal: true
# compiled: true
# typed: true
# run_filecheck: OPT

x = "hello, world"

# OPT: sorbet_int_rb_str_start_with
p x.start_with?()

# OPT: sorbet_int_rb_str_start_with
p x.start_with?("he")

# OPT: sorbet_int_rb_str_start_with
p x.start_with?("no")

# OPT: sorbet_int_rb_str_start_with
p x.start_with?("no", "he")

# OPT: sorbet_int_rb_str_start_with
p x.start_with?(/hel+o/)

# OPT: sorbet_int_rb_str_start_with
p x.start_with?(/no/, /hel+o/)

# OPT: sorbet_int_rb_str_start_with
p x.start_with?("no", /hel+o/)
