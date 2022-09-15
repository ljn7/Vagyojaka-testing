# https://5000-rohitbisht-me-8sqwd0i2h0v.ws-eu63.gitpod.io
import requests
import os
import sys
file=sys.argv[1]
file2=sys.argv[2]
with open(file, 'rb') as f:
    r = requests.post('https://5000-rohitbisht-me-8sqwd0i2h0v.ws-eu63.gitpod.io', files = {'file': open(file, 'rb')})
    with open(file2,'w',encoding='utf-8')as t:
      t.write(r.text)
