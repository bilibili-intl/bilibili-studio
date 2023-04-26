#!python3
# -*- coding: utf-8 -*-
# 0xCCCCCCCC

import re
import shlex
import subprocess
import sys

FIRST_REV = ''
LAST_REV = ''

def run(cmd):
    #print('-> ' + cmd)
    return subprocess.check_output(shlex.split(cmd)).decode('utf-8')

def query_authors():
    return list(map(lambda l: l.strip().split('\t')[1],
                    run('git shortlog -sn {}^..{}'.format(FIRST_REV, LAST_REV)).splitlines()))

def SumCommits(records):
    insertion_num = 0
    deletion_num = 0

    for stat in records:
        mr = re.search(r'(\d+)(?= insertion)', stat)
        if mr:
            insertion_num += int(mr.groups()[0])

        mr = re.search(r'(\d+)(?= deletion)', stat)
        if mr:
            deletion_num += int(mr.groups()[0])

    return insertion_num, deletion_num

def print_analyzed(results):
    print('{0:>20}Insertion{0:>10}Deletion{0:>10}Total'.format(' '))
    for author, records in results.items():
        print('{1:<20}{2:>6}{0:<10}{3:>6}{0:<10}{4:>6}'.format(' ', author, records[0], records[1], records[2]))

def main():
    if len(sys.argv) < 3:
        print('No full range specified! Analyze entire history for current branch.')
        return

    global FIRST_REV, LAST_REV
    FIRST_REV = sys.argv[1]
    LAST_REV = sys.argv[2]

    authors = query_authors()
    results = {}
    for author in authors:
        commits_query = 'git log {}^..{} --shortstat --author="{}"'.\
                            format(FIRST_REV, LAST_REV, author)
        commits = list(filter(lambda s: re.search('files? changed', s),
                              run(commits_query).splitlines()))
        total_insertions, total_deletions = SumCommits(commits)

        excluded_query = commits_query + ' --grep="Merge commits"'
        excluded_commits = list(filter(lambda s: re.search('files? changed', s),
                                       run(excluded_query).splitlines()))
        excluded_insertions, excluded_deletions = SumCommits(excluded_commits)

        total_insertions -= excluded_insertions
        total_deletions -= excluded_deletions

        results[author] = (total_insertions, total_deletions, total_insertions + total_deletions)

    print_analyzed(results)

if __name__ == '__main__':
    main()
