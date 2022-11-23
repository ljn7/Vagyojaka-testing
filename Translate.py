import sys
from typing import final
import requests

final_translation=""
with open(sys.argv[1],'r')as t:

  for line in t:

    payload = {"sentence": line}
    req = requests.post('https://udaaniitb2.aicte-india.org:8000/udaan_project_layout/translate/en/hi'.format("math,phy", "1"), data = payload, verify=False)
    final_translation+=req.json()['translation']+"\n"

with open(sys.argv[2],'w',encoding='utf-8')as tr:
  tr.write(final_translation)
# for line in final_translation:
#         print('<line timestamp="" speaker="Speaker_1">\n')
#         # reading each word        
#         for word in line.split():
#             # displaying the words           
#             print('<word timestamp="">')
#             print(word)
#             print('</word>\n')


with open(sys.argv[5],'w',encoding='utf-8')as t:
  t.write('<?xml version="1.0" encoding="UTF-8"?>\n')
  t.write('<transcript lang="hindi">\n')
    # reading each line 
  with open(sys.argv[2],'r',encoding='utf-8')as tr:
    for line in tr:
        t.write('<line timestamp="" speaker="Speaker_1">\n')
        # reading each word        
        for word in line.split():
            # displaying the words           
            t.write('<word timestamp="">')
            t.write(word)
            t.write('</word>\n')
        t.write('</line>\n')
    t.write('</transcript>\n')


count=[]
cn1=0
cn2=0
fu=""
with open (sys.argv[4],"r",encoding='utf-8') as re:
    with open (sys.argv[5],"r",encoding='utf-8') as wr:
        re1 = re.readline()
        wr1 = wr.readline()
        while re1 !="":
          if "line timestamp" in re1:
              cn1+=1
              count.append(re1)
          re1 = re.readline()

        while  wr1 != "":
            
            if "line timestamp" in wr1:
              cn2+=1
              wr1=count[cn2-1]
            fu+=wr1
            wr1 = wr.readline()

with open (sys.argv[3],"w",encoding='utf-8') as wx:
  wx.write(fu)
