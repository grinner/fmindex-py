"""
NGG NAG
5 prime 
10 bases

"""
from os.path import join as pjoin
import fmindex as fi
from totafo import fasta

root_path = "/Users/nick/Dropbox/"

strain_path = pjoin(root_path, "ecoli_strain_culling", "strain_sequences")
strain_files = ["Crooks_ATCC-8739.fasta", 
                "DH1_ATCC-33849.fasta", 
                "DH10B.fasta", 
                "MG1655.fasta"]


find_list = ["GG", "CC", "AG", "CT"]
fn = pjoin(strain_path, strain_files[0])
d = fasta.parseFasta(fn)
rec_body = list(d.values())[0]
print("setting up:", fn)
# for rec_id, rec_body in fasta.parseFastaGen(fn):
# print(rec_body)
fmi = fi.FMIndex(text=rec_body)
# fmi.save(pjoin(strain_path, "test"))
# fmi = fi.FMIndex(from_fmi_file=pjoin(strain_path, "test"))
print("finding")
for item in find_list:
    print("found", fmi.count(item), "occurences of", item)
    # ind = fmi.locate(find)
    # print("here they are")
    # for i in ind:
    #     print(i, fmi.extract(i, length=10))