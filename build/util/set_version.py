#!python3
# -*- coding: utf-8 -*-
# 0xCCCCCCCC

import argparse
import fileinput
import re

from os import path

def parse_commandline():
    cmd_parser = argparse.ArgumentParser()
    cmd_parser.add_argument('new_ver')
    return cmd_parser.parse_args()

def normalize_version_components(ver_components):
    if not ver_components:
        raise ValueError('Incorrect given version')

    for v in ver_components:
        if not str.isdigit(v):
            raise ValueError('Illegal characters!')

    digit_cnt = len(ver_components)
    if digit_cnt < 3:
        print('[*] Less than 3 version digits, will repair with digit 0!')
        ver_components.extend(['0' for i in range(3 - digit_cnt)])
    elif digit_cnt > 3:
        print('[*] More than 3 version digits are given. The redundant digits will be ignored!')
        ver_components = ver_components[:3]

    return ver_components

def update_version_file(proj_dir, ver_nums):
    version_path = path.join(proj_dir, 'bililive', 'VERSION')
    print('[*] Updating %s' % version_path)
    with fileinput.input(version_path, inplace=True) as fp:
        for i, line in enumerate(fp):
            if i < 3:
                pos = line.index('=') + 1
                line = line[:pos] + ver_nums[i] + '\n'
            print(line, end='')

def update_obs_config_file(proj_dir, ver_nums):
    obs_config_path = path.join(proj_dir, 'obs/obs_core/config/obsconfig.h')
    print('[*] Updating %s' % obs_config_path)
    pattern = re.compile(r'LiveHime/\d+\.\d+\.\d+')
    target_tag = 'LiveHime/%s' % '.'.join(ver_nums)
    with fileinput.input(obs_config_path, inplace=True) as fp:
        for line in fp:
            line = pattern.sub(target_tag, line)
            print(line, end='')

def main():
    args = parse_commandline()
    components = normalize_version_components([cp for cp in args.new_ver.strip().split('.') if cp])
    print('[*] Normalized target version: %s' % '.'.join(components))

    proj_dir = path.dirname(path.dirname(path.dirname(path.abspath(__file__))))

    update_version_file(proj_dir, components)
    update_obs_config_file(proj_dir, components)

    print('[*] Done')

"""
usage: set_version.py major.minor.bugfix, no `v` prefixes.
build-no is set by jenkins.
"""
if __name__ == '__main__':
    main()
