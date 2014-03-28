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
