import os
import sys

def test():
  os.system("make clean")
  os.system("make")
  if os.path.isfile("centralized_results.txt"):
    os.remove("centralized_results.txt")
  for i in range(2,9):
    str1 = "./test_centralized"
    str1 = str1 + " " +str(i) + " >> centralized_results.txt"
    print str1
    os.system(str1)
  os.system("make clean")

def main(args):
  test()

if __name__ == '__main__':
  main(sys.argv)
