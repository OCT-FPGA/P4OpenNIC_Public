#!/usr/bin/python

#THis script will take traffic.user as input and convert it to text2pcap compatiable txt
import sys
import os
from tkinter import W

pktCounter = 0
wordCount = 0 
lineCount = 0
rdFileName = str(sys.argv[1])
wrFileName = str(sys.argv[2])
tempTxtName = wrFileName + ".txt"
with open(rdFileName, 'r') as file:
    lines = [line.strip() for line in file if line.strip()]
wrFile = open(tempTxtName,'w')
for line in lines:
    if (line[-1] == ";"):
        line = line.rstrip(line[-1])
    words = line.split()
    if (len(words) ==0 ):
        continue
    if (words[0] == '%'):
        if(words[1] =='Packet'):
            if (pktCounter !=0):
                print("\n", file = wrFile)
            pktCounter +=1 
            wordCount = 0 
            lineCount = 0
            print("{:06d}".format(lineCount), end = " ", file=wrFile)
        continue
    for word in words:
        wordCount +=1
        print(word, end=" ", file=wrFile)
        if (wordCount % 16 == 0): 
            lineCount += 10
            print("\n{:06d}".format(lineCount), end = " ", file=wrFile)
wrFile.close()
cmd = "text2pcap " + tempTxtName + " " + wrFileName 
os.system(cmd)
cmd = "rm -rf " + tempTxtName
os.system(cmd)