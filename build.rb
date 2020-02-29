#!/usr/bin/env ruby
# The HAL library build script
#                                     ,--,                                    
#                                  ,---.'|                                    
#     ,---,.                  ,---,|   | :       ,---,                        
#   ,'  .'  \         ,--, ,`--.' |:   : |     .'  .' `\             ,---,    
# ,---.' .' |       ,'_ /| |   :  :|   ' :   ,---.'     \   __  ,-.,---.'|    
# |   |  |: |  .--. |  | : :   |  ';   ; '   |   |  .`\  |,' ,'/ /||   | :    
# :   :  :  /,'_ /| :  . | |   :  |'   | |__ :   : |  '  |'  | |' |:   : :    
# :   |    ; |  ' | |  . . '   '  ;|   | :.'||   ' '  ;  :|  |   ,':     |,-. 
# |   :     \|  | ' |  | | |   |  |'   :    ;'   | ;  .  |'  :  /  |   : '  | 
# |   |   . |:  | | :  ' ; '   :  ;|   |  ./ |   | :  |  '|  | '   |   |  / : 
# '   :  '; ||  ; ' |  | ' |   |  ';   : ;   '   : | /  ; ;  : |   '   : |: | 
# |   |  | ; :  | : ;  ; | '   :  ||   ,/    |   | '` ,/__|  , ;   |   | '/ : 
# |   :   /  '  :  `--'   \;   |.' '---'     ;   :  .'/  .\---'    |   :    | 
# |   | ,'   :  ,      .-./'---'             |   ,.'  \  ; |       /    \  /  
# `----'      `--`----'                      '---'     `--"        `-'----'   
#
# Created by Rory B. Bellows on 26/11/2017.
# Copyright © 2017-2019 George Watson. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# *   Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# *   Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
# *   Neither the name of the <organization> nor the
#     names of its contributors may be used to endorse or promote products
#     derived from this software without specific prior written permission.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL GEORGE WATSON BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

require 'rbconfig'
require 'json'

def puts_error msg
  STDERR.puts "! \e[31mERROR\e[0m: #{msg}"
end

def puts_abort msg
  STDERR.puts "! \e[31mERROR\e[0m: #{msg}"
  exit 1
end

def puts_warning msg
  puts "! \e[33mWARNING\e[0m: #{msg}"
end

$args = Hash[$*.map do |s|
  k, _ = s.scan /--?([^=\s]+)(?:=(\S+))?/
  k[1] = case k[1]
                when /^\d+$/
                  k[1].to_i
                when /^[Tt]rue$/
                  true 
                when /^[Ff]alse$/
                  false
                when /,/
                  k[1].split ','
                when nil
                  true
                else
                  k[1]
                end
  k
end]

$info = {:ver => {:maj => 0, :min => 0, :rev => 0},
        :os  => RbConfig::CONFIG['host_os'],
        :str => nil,
        :supported_backends => nil,
        :backend  => nil,
        :cc => 'cc',
        :layers => [],
        :flags => nil,
        :dependencies => []}
case RbConfig::CONFIG['host_os']
when /mswin|msys|mingw|cygwin|bccwin|wince|emc/
  # TODO
  puts_abort 'Windows needs to be reimplemented'
when /darwin|mac os/
  $info[:ver][:maj], $info[:ver][:min], $info[:ver][:rev] = `sw_vers | grep 'ProductVersion'`.split(' ')[1].split('.').map(&:to_i)
  $info[:str] = "Mac OS X #{$info[:ver][:maj]}.#{$info[:ver][:min]}.#{$info[:ver][:rev]}"
  if $info[:ver][:maj] < 10
    puts_warning 'Mac OS X < 10.0 is unsupported'
    $info[:supported_backends] = [:OPENGL, :CARBON]
  else
    if $info[:ver][:min] > 13 # Metal or OpenGL only
      $info[:supported_backends] = [:METAL, :OPENGL]
    elsif $info[:ver][:min] > 11 # Any backend
      $info[:supported_backends] = [:METAL, :OPENGL, :CARBON]
    else # OpenGL or Carbon only
      puts_warning 'Mac OS X > 10.0 & < 10.11 is unsupported at the moment'
      $info[:supported_backends] = [:OPENGL, :CARBON]
    end
  end
  $info[:flags] = ['-x objective-c', '-fno-common -fno-objc-arc']
when /linux|solaris|bsd/
  # TODO
  puts_abort 'Linux needs to be reimplemented'
else
  puts_abort 'Unknown OS, you\'re on your own'
end

if $args.key? 'backend'
  if $args['backend'].instance_of? String
    be_sym = $args['backend'].upcase.to_sym
    unless $info[:supported_backends].include? be_sym 
      puts_abort "Unsupported backend. #{$info[:str]} does not support \"#{$args['backend']}\""
    else
      $info[:backend] = be_sym
    end
  else
    puts_abort 'Invalid backend parameter, must be a string'
  end
  $args.reject! { |k, v| k == 'backend' }
else
  $info[:backend] = $info[:supported_backends][0]
end

$info[:dependencies] = case $info[:backend]
                       when :METAL
                         ['-framework Cocoa', '-framework AppKit', '-framework Metal', '-framework MetalKit']
                       when :OPENGL
                         ['-framework Cocoa', '-framework AppKit', '-framework OpenGL']
                       when :CARBON
                         ['-framework Cocoa', '-framework AppKit']
                       else
                         puts_abort 'Unsupported backend. I don\'t know how it got to this point'
                       end

if $args.key? 'cc'
  if $args['cc'].instance_of? String
    cc = $args['cc'].downcase
    puts_warning "Unsupported C compiler \"#{cc}\"" unless ["clang", "msvc", "gcc"].include? cc
    $info[:cc] = cc;
    $args.reject! { |k, v| k == 'cc' }
  else
    puts_abort 'Invalid C compiler parameter, must be a string'
  end
end

$valid_layers = ['graphics.h', 'audio.h', 'threads.h', 'sockets.h', 'filesystem.h']
def is_layer_valid? x
  $valid_layers.include? x and File.file? x
end

def add_h x
  x.end_with?('.h') ? x : x + '.h'  
end

if $args.key? 'layers'
  case $args['layers']
  when Array
    $args['layers'].map { |l| add_h l }.each do |l|
      puts_abort "Invalid layer \"#{l}\"" unless is_layer_valid? l
      $info[:layers].push l
    end
  when String
    l = add_h $args['layers']
    puts_abort "Invalid layer \"#{l}\"" unless is_layer_valid? l
    $info[:layers].push l
  else
    puts_abort 'Invalid layers parameter, must be an array of strings'
  end
  $args.reject! { |k, v| k == 'layers' }
else
  # TODO: Remove reject after testing finished
  $info[:layers] = $valid_layers.reject { |x| !is_layer_valid? x }
end

$valid_modes = ['make', 'info', 'json']
$mode = :make
if $args.key? 'mode'  
  if $args['mode'].instance_of? String
    m = $args['mode'].downcase
    if $valid_modes.include? m 
      $mode = m.to_sym
      $args.reject! { |k, v| k == 'mode' }
    else
      puts_abort "Invalid mode, \"#{m}\". Valid modes: #{$valid_modes}"
    end
  else
    puts_abort 'Invalid mode parameter, must be a string'
  end
end

$skip_flags = false
if $args.key? 'config'
  if $args['config'].instance_of? String
    if File.file? $args['config']
      begin
        $info = JSON.parse File.read($args['config'])
        $skip_flags = true
      rescue JSON::ParserError
        puts_abort "Invalid config \"#{$args['config']}\", failed to parse JSON"
      end
    else
    end
  end
end

unless $skip_flags
  $info[:flags] = $args.map do |k, v|
    case v
    when String 
      "-#{k}=#{v}"
    when Array
      "-#{k}=#{v.join ','}"
    else
      '-' + k
    end
  end
end

File.open "build/hal_h.h", "w" do |f|
  File.readlines("hal.h").each do |l|
    /^#include "(?<p>\S+)\.h"[\r\n]$/ =~ l
    unless p
      f.write l
    else
      start = false
      pp = p + '.h'
      puts_abort "Internal error: \"#{pp}\" doesn't exist (from hal.h)" unless File.file? pp
      stop = false
      File.readlines(pp)[57..-1].each do |ll|
        if stop
          stop = false if ll =~ /^#endif[\r\n]$/
        elsif ll =~ /^#if defined\(__cplusplus\)[\r\n]$/
          stop = true
        else
          f.write ll
        end
      end
    end
  end
end

$info[:layers].each do |f|
  ll, ls, le = [], 0, 0
  File.readlines(f).each_with_index do |l, i|
    ll.push l
    ls = i if l =~ /^#if defined\(HAL_IMPLEMENTATION\)$/
    le = i if l =~ /^#endif \/\/ HAL_IMPLEMENTATION$/
  end
  puts_abort "Internal error: \"#{f}\" has nothing to split" if not ls or not le or ls + 1 == le

  nf = 'build/' + f.gsub(/\.h$/, '.c')
  File.write nf, (["// Generated by build.rb\n", "#include \"#{f}\"\n\n"] + ll.slice!(ls..le)[1..-2]) * ''
  File.write 'build/' + f, ll * ''
end


case $mode
when :make
  File.open 'Makefile', 'w' do |fh|
    fh.write """# Generated by build.rb
CC=#{$info[:cc]}
DEPS=#{$info[:dependencies].join ' '}
FLAGS=#{$info[:flags].join ' '}
BACKEND=-DHAL_#{$info[:backend]}
LAYERS=#{$info[:layers].join ' '}
"""
  end
when :info
  puts """CC      = #{$info[:cc]}
DEPS    = #{$info[:dependencies].join ' '}
FLAGS   = #{$info[:flags].join ' '}
BACKEND = #{$info[:backend]}
LAYERS  = #{$info[:layers].join ' '}
#{$info[:cc]} #{$info[:flags].join ' '} #{$info[:dependencies].join ' '}"""
when :json
  puts $info.to_json
else
  puts_abort 'Invalid mode. I don\'t know how we got here'
end
