/*
 * errors.h -- define all known error codes
 *
 * You probably don't want to include this file. Including this file
 * means that adding a new error code to any phase will force a
 * rebuild of your file. Instead, include only the error file that
 * defines your phase's error codes.
 */

#include "core/errors/cfg.h"
#include "core/errors/desugar.h"
#include "core/errors/infer.h"
#include "core/errors/internal.h"
#include "core/errors/namer.h"
#include "core/errors/packager.h"
#include "core/errors/parser.h"
#include "core/errors/plugin.h"
#include "core/errors/resolver.h"
