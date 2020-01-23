# frozen_string_literal: true
# typed: true
# compiled: true
100000.times { Marshal.load(Marshal.dump(Time.now)) }
