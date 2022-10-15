
import requests
import os
import sys
file=sys.argv[1]
file2=sys.argv[2]
url='https://udaaniitb.aicte-india.org:8000/asr/'
with open(file, 'rb') as f:
    r = requests.post(url, files = {'file': open(file, 'rb')},verify=False)
    with open(file2,'w',encoding='utf-8')as t:
      t.write(r.text)

