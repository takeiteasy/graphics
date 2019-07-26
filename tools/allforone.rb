#!/usr/bin/env ruby

File.open "hal_h.h", "w" do |f|
  File.readlines("hal.h").each do |l|
    /^#include "(?<p>\S+)\.h"[\r\n]$/ =~ l
    unless p
      puts l
    else
      start = false
      pp = p + '.h'
      unless File.file? pp
        File.delete "hal_h.h"
        exit 1
      end
      stop = false
      File.readlines(pp)[57..-1].each do |ll|
        if stop
          stop = false if ll =~ /^#endif[\r\n]$/
        elsif ll =~ /^#if defined\(__cplusplus\)[\r\n]$/
          stop = true
        else
          puts ll
        end
      end
    end
  end
end
