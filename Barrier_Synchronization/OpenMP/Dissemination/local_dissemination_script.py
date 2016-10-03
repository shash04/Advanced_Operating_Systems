import os
import sys

def test():
  os.system("make clean")
  os.system("make")
  if os.path.isfile("OpenMP_dissemination_results.txt"):
    os.remove("OpenMP_dissemination_results.txt")
  for i in range(2,9):
    str1 = "./test_dissemination"
    str1 = str1 + " " +str(i) + " >> OpenMP_dissemination_results.txt"
    print str1
    os.system(str1)
  os.system("make clean")

def main(args):
  test()

if __name__ == '__main__':
  main(sys.argv)