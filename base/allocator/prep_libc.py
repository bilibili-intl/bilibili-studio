#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This script takes libcmt.lib for VS2013 and removes the allocation related
# functions from it.
#
# Usage: prep_libc.py <VCLibDir> <OutputDir> <arch> [<environment_file>]
#
# VCLibDir is the path where VC is installed, something like:
#    C:\Program Files\Microsoft Visual Studio 8\VC\lib
#
# OutputDir is the directory where the modified libcmt file should be stored.
# arch is one of: 'ia32', 'x86' or 'x64'. ia32 and x86 are synonyms.
#
# If the environment_file argument is set, the environment variables in the
# given file will be used to execute the VC tools. This file is in the same
# format as the environment block passed to CreateProcess.
import os
import shutil
import subprocess
import sys
def run(command, env_dict):
  """Run |command|.  If any lines that match an error condition then
      terminate.
  The env_dict, will be used for the environment. None can be used to get the
  default environment."""
  error = 'cannot find member object'
  # Need shell=True to search the path in env_dict for the executable.
  popen = subprocess.Popen(
      command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True,
      env=env_dict)
  out, _ = popen.communicate()
  for line in out.splitlines():
    print line
    if error and line.find(error) != -1:
      print 'prep_libc.py: Error stripping object from C runtime.'
      sys.exit(1)
def main():
  bindir = 'SELF_X86'
  objdir = 'INTEL'
  vs_install_dir = sys.argv[1]
  outdir = sys.argv[2]
  if "x64" in sys.argv[3]:
    bindir = 'SELF_64_amd64'
    objdir = 'amd64'
    vs_install_dir = os.path.join(vs_install_dir, 'amd64')
  if len(sys.argv) == 5:
    env_pairs = open(sys.argv[4]).read()[:-2].split('\0')
    env_dict = dict([item.split('=', 1) for item in env_pairs])
  else:
    env_dict = None  # Use the default environment.
  output_lib = os.path.join(outdir, 'libcmt.lib')
  shutil.copyfile(os.path.join(vs_install_dir, 'libcmt.lib'), output_lib)
  #shutil.copyfile(os.path.join(vs_install_dir, 'libcmt.pdb'),
  #                os.path.join(outdir, 'libcmt.pdb'))
  cvspath = 'f:\\binaries\\Intermediate\\vctools\\crt_bld\\' + bindir + \
      '\\crt\\prebuild\\build\\' + objdir + '\\mt_obj\\nativec\\\\';
  cppvspath = 'f:\\binaries\\Intermediate\\vctools\\crt_bld\\' + bindir + \
      '\\crt\\prebuild\\build\\' + objdir + '\\mt_obj\\nativecpp\\\\';
  cobjfiles = ['malloc', 'free', 'realloc', 'heapinit', 'calloc', 'recalloc',
      'calloc_impl', 'msize']
  cppobjfiles = ['new', 'new2', 'delete', 'delete2', 'new_mode', 'newopnt',
      'newaopnt']
  for obj in cobjfiles:
    cmd = ('lib /nologo /ignore:4006,4221,4014 /remove:%s%s.obj %s' %
           (cvspath, obj, output_lib))
    run(cmd, env_dict)
  for obj in cppobjfiles:
    cmd = ('lib /nologo /ignore:4006,4221,4014 /remove:%s%s.obj %s' %
           (cppvspath, obj, output_lib))
    run(cmd, env_dict)
if __name__ == "__main__":
  sys.exit(main())