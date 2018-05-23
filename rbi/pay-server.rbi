# This file contains pay-server specific hacks that have to be in the
# sorbet pre-built image for whatever reason.
# typed: strict

# pay-server defines classes and modules under the `M` alias. In order
# for that to work, it has to be pre-defined before we enter the Namer
# over pay-server. Bake it in here.
module Opus::DB::Model; end
::M = Opus::DB::Model
