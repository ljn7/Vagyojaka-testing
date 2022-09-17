# https://5000-rohitbisht-me-8sqwd0i2h0v.ws-eu63.gitpod.io
import requests
import os
import sys
file=sys.argv[1]
file2=sys.argv[2]
url='https://5000-rohitbisht-asrbackend-mh87fvqbwpf.ws-us65.gitpod.io'
with open(file, 'rb') as f:
    r = requests.post(url, files = {'file': open(file, 'rb')})
    with open(file2,'w',encoding='utf-8')as t:
      t.write(r.text)
