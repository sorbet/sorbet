# frozen_string_literal: true
# typed: true
# compiled: true

# TODO(jez) FileCheck assertions

current = Thread.current
p(current.class)

p(current[:my_key] = 'my value')
p(current['another key'] = 646)

p(current[:my_key])
p(current['another key'])
