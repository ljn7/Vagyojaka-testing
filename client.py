import requests
import os
import sys
file=sys.argv[1]
file2=sys.argv[2]
url='https://udaaniitb.aicte-india.org:8000/asr/transcript'

r = requests.post(url, files = {'file': open(file, 'rb')},verify=False)
r2= requests.get(url+'/'+r.text,verify=False)
while(r2.text!='SUCCESS'):
  r2= requests.get(url+'/'+r.text,verify=False)
r3=requests.get(url+'/'+r.text+'/result',verify=False)
with open(file2,'w',encoding='utf-8')as t:
  t.write(r3.text)
