"""
Need test coverage for large texts!!!
and to improve this existing coverage
"""
import fmindex as fi

myi = fi.FMIndex(text="I Like nachos and Cheese")

print(myi.count("ees"))
print(myi.count("e"))
print(myi.count("z"))

print(myi.locate("e"))
ind = myi.locate("n")

print(myi.display("e", 1))

print(myi.extract(ind[0], length=6))

print("doing DNA now")
myi2 = fi.FMIndex(from_text_file="seg11.txt")
# myi2 = fi.FMIndex(from_text_file="cupcake.txt")
find = "GCGC"
print("found", myi2.count(find), "occurences of", find)
ind = myi2.locate(find)
print("here they are")
for i in ind:
    print(i, myi2.extract(i, length=10))