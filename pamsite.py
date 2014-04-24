"""
NGG NAG
5 prime 
10 bases

"""
from os.path import join as pjoin
import fmindex as fi
from totafo import fasta
import re
root_path = "/Users/nick/Dropbox/"

strain_path = pjoin(root_path, "ecoli_strain_culling", "strain_sequences")
strain_files = ["Crooks_ATCC-8739.fasta", 
                "DH1_ATCC-33849.fasta", 
                "DH10B.fasta", 
                "MG1655.fasta"]

def levenshtein(s1, s2):
    l1 = len(s1)
    l2 = len(s2)

    matrix = [i for i in range(l1 + 1)] * (l2 + 1)
    for zz in range(l2 + 1):
        matrix[zz] = [i for i in range(zz,zz + l1 + 1)]
    for zz in range(0,l2):
        for sz in range(0,l1):
            if s1[sz] == s2[zz]:
                matrix[zz+1][sz+1] = min(matrix[zz+1][sz] + 1, matrix[zz][sz+1] + 1, matrix[zz][sz])
            else:
                matrix[zz+1][sz+1] = min(matrix[zz+1][sz] + 1, matrix[zz][sz+1] + 1, matrix[zz][sz] + 1)
    return matrix[l2][l1]
# end def

find_list = ["GG"]#, "CC", "AG", "CT"]
potentials = []
FRAG_SIZE = 13

# regex_strs = ["([ATGC]{21}GG)",
#                 "([ATGC]{21}CC)",
#                 "([ATGC]{21}AG)",
#                 "([ATGC]{21}CT)"]
regex_str = re.compile("([ATGC]{21}(?:GG|CC|AG|CT))")
# regex_str = re.compile("GG")
records = []
for i, fn in enumerate(strain_files):

    path = pjoin(strain_path, fn)
    d = fasta.parseFasta(path)
    rec_body = d[0][1]
    matches = []
    i = 0
    pos = 0
    tmp = rec_body
    while True:
        match = regex_str.search(tmp, pos)
        if match is None:
            break
        else:
            matches.append(match)
            pos = match.end(0) - 1
    print("regex total", len(matches))
    potentials.append(matches)
    records.append(rec_body)

# find alignment between genomes
reference_matches = potentials[3]
print(len(reference_matches), len(records))
i = -1
offsets = []
for i, match_frag in enumerate(reference_matches):
    offset_str = match_frag.group(0)
    offset_idx = match_frag.start(0)
    regex_offset = re.compile(offset_str)
    j = 0
    offsets = []
    # print(offset_str, i)
    nomatch = False
    for rec in records[0:3]:
        m = regex_offset.search(rec)
        if m is None:
            # print("no match", j)
            nomatch = True
            break
        offsets.append(m.start(0))
    if nomatch:
        continue
    else:
        print("breaking")
        break

print("frag index:", i) 
print(offset_idx, offsets)
offsets.append(offset_idx)
new_records = []
new_potentials = []
for i, rec in enumerate(records):
    rec_body = rec[offsets[i]:] + rec[:offsets[i]]
    matches = []
    pos = 0
    tmp = rec_body
    while True:
        match = regex_str.search(tmp, pos)
        if match is None:
            break
        else:
            matches.append(match)
            pos = match.end(0) - 1
    # print("regex total", len(matches))
    new_potentials.append(matches)
    new_records.append(rec_body)

# just a check
for i in range(4):
    print(i)
    for p in new_potentials:
        print(p[i].group(0))

min_pam_length = min([len(p) for p in new_potentials])
newref = new_potentials[3]
solutions = []
for i in range(min_pam_length):
    seq = newref[i].group(0)
    count1 = 0
    count2 = 0
    seqtest_ref = None
    for p in new_potentials[0:3]:
        seqtest = p[i].group(0)
        if seq != seqtest:
            count1 += 1
            if seqtest_ref is None:
                seqtest_ref = seqtest
            elif levenshtein(seqtest_ref, seqtest) < 3:
                count2 += 1
    if count1 == 3 and count2 == 2:
        solutions.append(i)

print("a solution at ", i, "and total count:", len(solutions), "out of a possible:", min_pam_length)
for p in new_potentials:
    print(p[i].group(0))
