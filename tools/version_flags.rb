#!/usr/bin/env ruby

maj, min, rev = File.readlines("VERSION")[0].split '.'
rev = rev[0..-2] if rev[-1] == '\n'
puts "-DHAL_VERSION_MAJOR=#{maj} -DHAL_VERSION_MINOR=#{min} -DHAL_VERSION_REV=#{rev} -DHAL_VERSION_GIT=\"#{`git rev-parse --short HEAD`[0..-2]}\""
