#!/usr/bin/env ruby
# encoding: utf-8

COMPIL_OPTS = '-x c++ --std=c++11'.freeze
CHECKS = %w[
  cert-*
  google-build-explicit-make-pair
  google-default-arguments
  google-explicit-constructor
  google-readability-casting
  google-runtime-member-string-references
  google-runtime-int
  hicpp-avoid-goto
  hicpp-exception-baseclass
  hicpp-member-init
  hicpp-no-array-decay
  hicpp-no-assembler
  hicpp-no-malloc
  hicpp-signed-bitwise
  hicpp-special-member-functions
  hicpp-vararg
  modernize-*
  performance-*
  readability-*
].freeze

BLACKLIST = %w[
  src/gethclient.h
  src/kadclient.h
].freeze
TO_CHECK = Dir.glob('src/*.{cpp,h}') - BLACKLIST

exec("clang-tidy #{TO_CHECK.join(' ')} "\
     "-checks='#{CHECKS.join(',')}' -- #{COMPIL_OPTS}")
