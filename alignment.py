# Copyright 2017-2018 Ben Lambert

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#     http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# import sys
# print(sys.argv[1])

from __future__ import division

from functools import reduce
from collections import defaultdict
from edit_distance import SequenceMatcher

from termcolor import colored
import sys
# print(sys.argv[1])
import json
from pathlib import Path

# Some defaults
print_instances_p = False
print_errors_p = False
files_head_ids = False
files_tail_ids = False
confusions = False
min_count = 0
wer_vs_length_p = True

# For keeping track of the total number of tokens, errors, and matches
ref_token_count = 0
error_count = 0
match_count = 0
counter = 0
sent_error_count = 0

# For keeping track of word error rates by sentence length
# this is so we can see if performance is better/worse for longer
# and/or shorter sentences
lengths = []
error_rates = []
wer_bins = defaultdict(list)
wer_vs_length = defaultdict(list)
# Tables for keeping track of which words get confused with one another
insertion_table = defaultdict(int)
deletion_table = defaultdict(int)
substitution_table = defaultdict(int)
# These are the editdistance opcodes that are condsidered 'errors'
error_codes = ['replace', 'delete', 'insert']


# TODO - rename this function.  Move some of it into evaluate.py?
def main(args):
   
    global counter
    set_global_variables(args)

    counter = 0
    # Loop through each line of the reference and hyp file
    for ref_line, hyp_line in zip(args.ref, args.hyp):
        processed_p = process_line_pair(ref_line, hyp_line, case_insensitive=args.case_insensitive,
                                        remove_empty_refs=args.remove_empty_refs)
        if processed_p:
            counter += 1
    if confusions:
        print_confusions()
    if wer_vs_length_p:
        print_wer_vs_length()
    # Compute WER and WRR
    if ref_token_count > 0:
        wrr = match_count / ref_token_count
        wer = error_count / ref_token_count
    else:
        wrr = 0.0
        wer = 0.0
    # Compute SER
    ser = sent_error_count / counter if counter > 0 else 0.0
    # print('Sentence count: {}'.format(counter))
    # print('WER: {:10.3%} ({:10d} / {:10d})'.format(wer, error_count, ref_token_count))
    # print('WRR: {:10.3%} ({:10d} / {:10d})'.format(wrr, match_count, ref_token_count))
    # print('SER: {:10.3%} ({:10d} / {:10d})'.format(ser, sent_error_count, counter))


def process_line_pair(ref_line, hyp_line, case_insensitive=False, remove_empty_refs=False):
    
    # I don't believe these all need to be global.  In any case, they shouldn't be.
    global error_count
    global match_count
    global ref_token_count
    global sent_error_count

    # Split into tokens by whitespace
    ref = ref_line.split()
    hyp = hyp_line.split()
    id_ = None

    # If the files have IDs, then split the ID off from the text
    if files_head_ids:
        id_ = ref[0]
        ref, hyp = remove_head_id(ref, hyp)
    elif files_tail_ids:
        id_ = ref[-1]
        ref, hyp = remove_tail_id(ref, hyp)

    if case_insensitive:
        ref = list(map(str.lower, ref))
        hyp = list(map(str.lower, hyp))
    if remove_empty_refs and len(ref) == 0:
        return False

    # Create an object to get the edit distance, and then retrieve the
    # relevant counts that we need.
    sm = SequenceMatcher(a=ref, b=hyp)
    errors = get_error_count(sm)
    matches = get_match_count(sm)
    ref_length = len(ref)

    # Increment the total counts we're tracking
    error_count += errors
    match_count += matches
    ref_token_count += ref_length

    if errors != 0:
        sent_error_count += 1

    # If we're keeping track of which words get mixed up with which others, call track_confusions
    if confusions:
        track_confusions(sm, ref, hyp)

    # If we're printing instances, do it here (in roughly the align.c format)
    if print_instances_p or (print_errors_p and errors != 0):
        print_instances(ref, hyp, sm, id_=id_)

    # Keep track of the individual error rates, and reference lengths, so we
    # can compute average WERs by sentence length
    lengths.append(ref_length)
    error_rate = errors * 1.0 / len(ref) if len(ref) > 0 else float("inf")
    error_rates.append(error_rate)
    wer_bins[len(ref)].append(error_rate)
    return True

def set_global_variables(args):
   
    global print_errors_p
    global files_head_ids
    global files_tail_ids
    global confusions
    global min_count
    global wer_vs_length_p
    # Put the command line options into global variables.
    print_instances_p = args.print_instances
    print_errors_p = args.print_errors
    files_head_ids = args.head_ids
    files_tail_ids = args.tail_ids
    confusions = args.confusions
    min_count = args.min_word_count
    wer_vs_length_p = args.print_wer_vs_length

def remove_head_id(ref, hyp):
   
    ref_id = ref[0]
    hyp_id = hyp[0]
    if ref_id != hyp_id:
        print('Reference and hypothesis IDs do not match! '
              'ref="{}" hyp="{}"\n'
              'File lines in hyp file should match those in the ref file.'.format(ref_id, hyp_id))
        exit(-1)
    ref = ref[1:]
    hyp = hyp[1:]
    return ref, hyp

def remove_tail_id(ref, hyp):
    
    ref_id = ref[-1]
    hyp_id = hyp[-1]
    if ref_id != hyp_id:
        print('Reference and hypothesis IDs do not match! '
              'ref="{}" hyp="{}"\n'
              'File lines in hyp file should match those in the ref file.'.format(ref_id, hyp_id))
        exit(-1)
    ref = ref[:-1]
    hyp = hyp[:-1]
    return ref, hyp

def print_instances(ref, hyp, sm, id_=None):
   
    print_diff(sm, ref, hyp)
    if id_:
        print(('SENTENCE {0:d}  {1!s}'.format(counter + 1, id_)))
    else:
        print('SENTENCE {0:d}'.format(counter + 1))
    # Handle cases where the reference is empty without dying
    if len(ref) != 0:
        correct_rate = sm.matches() / len(ref)
        error_rate = sm.distance() / len(ref)
    elif sm.matches() == 0:
        correct_rate = 1.0
        error_rate = 0.0
    else:
        correct_rate = 0.0
        error_rate = sm.matches()
    print('Correct          = {0:6.1%}  {1:3d}   ({2:6d})'.format(correct_rate, sm.matches(), len(ref)))
    print('Errors           = {0:6.1%}  {1:3d}   ({2:6d})'.format(error_rate, sm.distance(), len(ref)))

def track_confusions(sm, seq1, seq2):
   
    opcodes = sm.get_opcodes()
    for tag, i1, i2, j1, j2 in opcodes:
        if tag == 'insert':
            for i in range(j1, j2):
                word = seq2[i]
                insertion_table[word] += 1
        elif tag == 'delete':
            for i in range(i1, i2):
                word = seq1[i]
                deletion_table[word] += 1
        elif tag == 'replace':
            for w1 in seq1[i1:i2]:
                for w2 in seq2[j1:j2]:
                    key = (w1, w2)
                    substitution_table[key] += 1

def print_confusions():
    print(sys.argv[4])
    # if len(insertion_table) > 0:
    #     print('INSERTIONS:')
    #     for item in sorted(list(insertion_table.items()), key=lambda x: x[1], reverse=True):
    #         if item[1] >= min_count:
    #             print('{0:20s} {1:10d}'.format(*item))
    # if len(deletion_table) > 0:
    #     print('DELETIONS:')
    #     for item in sorted(list(deletion_table.items()), key=lambda x: x[1], reverse=True):
    #         if item[1] >= min_count:
    #             print('{0:20s} {1:10d}'.format(*item))
    if len(substitution_table) > 0:
        # print('SUBSTITUTIONS:')
        fle = Path(sys.argv[4])
        fle.touch(exist_ok=True)
        with open(sys.argv[4], 'r') as f:
            data = json.load(f)
        
        for [w1, w2], count in sorted(list(substitution_table.items()), key=lambda x: x[1], reverse=True):
            if count >= min_count:
                print('{0} : {1:20s}  '.format(w1, w2))
                not_here=True
                data.setdefault(w1, [])
                for i in data[w1]:
                    if w2==i:
                        not_here=False
                        break
                if(not_here):
                    data.setdefault(w1, []).append(w2)
        with open(sys.argv[4], "w") as write_file:
            json.dump(data, write_file)

# TODO - For some reason I was getting two different counts depending on how I count the matches,
# so do an assertion in this code to make sure we're getting matching counts.
# This might slow things down.
def get_match_count(sm):
    matches = None
    matches1 = sm.matches()
    matching_blocks = sm.get_matching_blocks()
    matches2 = reduce(lambda x, y: x + y, [x[2] for x in matching_blocks], 0)
    assert matches1 == matches2
    matches = matches1
    return matches


def get_error_count(sm):
    opcodes = sm.get_opcodes()
    errors = [x for x in opcodes if x[0] in error_codes]
    error_lengths = [max(x[2] - x[1], x[4] - x[3]) for x in errors]
    return reduce(lambda x, y: x + y, error_lengths, 0)

# TODO - This is long and ugly.  Perhaps we can break it up?
# It would make more sense for this to just return the two strings...
def print_diff(sm, seq1, seq2, prefix1='REF:', prefix2='HYP:', suffix1=None, suffix2=None):
    ref_tokens = []
    hyp_tokens = []
    opcodes = sm.get_opcodes()
    for tag, i1, i2, j1, j2 in opcodes:
        # If they are equal, do nothing except lowercase them
        if tag == 'equal':
            for i in range(i1, i2):
                ref_tokens.append(seq1[i].lower())
            for i in range(j1, j2):
                hyp_tokens.append(seq2[i].lower())
        # For insertions and deletions, put a filler of '***' on the other one, and
        # make the other all caps
        elif tag == 'delete':
            for i in range(i1, i2):
                ref_token = colored(seq1[i].upper(), 'red')
                ref_tokens.append(ref_token)
            for i in range(i1, i2):
                hyp_token = colored('*' * len(seq1[i]), 'red')
                hyp_tokens.append(hyp_token)
        elif tag == 'insert':
            for i in range(j1, j2):
                ref_token = colored('*' * len(seq2[i]), 'red')
                ref_tokens.append(ref_token)
            for i in range(j1, j2):
                hyp_token = colored(seq2[i].upper(), 'red')
                hyp_tokens.append(hyp_token)
        # More complicated logic for a substitution
        elif tag == 'replace':
            seq1_len = i2 - i1
            seq2_len = j2 - j1
            # Get a list of tokens for each
            s1 = list(map(str.upper, seq1[i1:i2]))
            s2 = list(map(str.upper, seq2[j1:j2]))
            # Pad the two lists with False values to get them to the same length
            if seq1_len > seq2_len:
                for i in range(0, seq1_len - seq2_len):
                    s2.append(False)
            if seq1_len < seq2_len:
                for i in range(0, seq2_len - seq1_len):
                    s1.append(False)
            assert len(s1) == len(s2)
            # Pair up words with their substitutions, or fillers
            for i in range(0, len(s1)):
                w1 = s1[i]
                w2 = s2[i]
                # If we have two words, make them the same length
                if w1 and w2:
                    if len(w1) > len(w2):
                        s2[i] = w2 + ' ' * (len(w1) - len(w2))
                    elif len(w1) < len(w2):
                        s1[i] = w1 + ' ' * (len(w2) - len(w1))
                # Otherwise, create an empty filler word of the right width
                if not w1:
                    s1[i] = '*' * len(w2)
                if not w2:
                    s2[i] = '*' * len(w1)
            s1 = map(lambda x: colored(x, 'red'), s1)
            s2 = map(lambda x: colored(x, 'red'), s2)
            ref_tokens += s1
            hyp_tokens += s2
    if prefix1: ref_tokens.insert(0, prefix1)
    if prefix2: hyp_tokens.insert(0, prefix2)
    if suffix1: ref_tokens.append(suffix1)
    if suffix2: hyp_tokens.append(suffix2)
    print(' '.join(ref_tokens))
    print(' '.join(hyp_tokens))

def mean(seq):
    return float(sum(seq)) / len(seq) if len(seq) > 0 else float('nan')

def print_wer_vs_length():
    avg_wers = {length: mean(wers) for length, wers in wer_bins.items()}
    for length, avg_wer in sorted(avg_wers.items(), key=lambda x: (x[1], x[0])):
        print('{0:5d} {1:f}'.format(length, avg_wer))
    print('')


import argparse
def get_parser():
    """Parse the CLI args."""
    parser = argparse.ArgumentParser(description='Evaluate an ASR transcript against a reference transcript.')
    parser.add_argument('ref', type=argparse.FileType('r'), help='Reference transcript filename')
    parser.add_argument('hyp', type=argparse.FileType('r'), help='ASR hypothesis filename')
    parser.add_argument('wit', help='ASR hypothesis filename')
    print_args = parser.add_mutually_exclusive_group()
    print_args.add_argument('-i', '--print-instances', action='store_true',
                            help='Print all individual sentences and their errors.')
    print_args.add_argument('-r', '--print-errors', action='store_true',
                            help='Print all individual sentences that contain errors.')
    parser.add_argument('--head-ids', action='store_true',
                        help='Hypothesis and reference files have ids in the first token? (Kaldi format)')
    parser.add_argument('-id', '--tail-ids', '--has-ids', action='store_true',
                        help='Hypothesis and reference files have ids in the last token? (Sphinx format)')
    parser.add_argument('-c', '--confusions', action='store_true', help='Print tables of which words were confused.')
    parser.add_argument('-p', '--print-wer-vs-length', action='store_true',
                        help='Print table of average WER grouped by reference sentence length.')
    parser.add_argument('-m', '--min-word-count', type=int, default=1, metavar='count',
                        help='Minimum word count to show a word in confusions (default 1).')
    parser.add_argument('-a', '--case-insensitive', action='store_true',
                        help='Down-case the text before running the evaluation.')
    parser.add_argument('-e', '--remove-empty-refs', action='store_true',
                        help='Skip over any examples where the reference is empty.')

    return parser
parser = get_parser()
args = parser.parse_args()

main(args)