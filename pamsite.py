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

for p in new_potentials:
    print(p[0].group(0))

