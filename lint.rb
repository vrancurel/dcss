#!/usr/bin/env ruby
# encoding: utf-8

COMPIL_OPTS = '-x c++ --std=c++11'.freeze
CHECKS = %w[
].freeze

BLACKLIST = %w[
  src/gethclient.h
  src/kadclient.h
].freeze
TO_CHECK = Dir.glob('src/*.{cpp,h}') - BLACKLIST

exec("clang-tidy #{TO_CHECK.join(' ')} "\
     "-checks='#{CHECKS.join(',')}' -- #{COMPIL_OPTS}")
