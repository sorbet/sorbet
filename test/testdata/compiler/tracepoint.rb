# frozen_string_literal: true
# typed: true
# compiled: true

# This catches regressions related to how we populate
# `iseq->body->iseq_compiled` in sorbet_allocateRubyStackFrame, as producing an
# invalid iseq_compiled entry will cause ruby crash when computing line numbers,
# or installing traces.
TracePoint.trace(:line) {}
