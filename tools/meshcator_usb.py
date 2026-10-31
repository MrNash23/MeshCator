#!/usr/bin/env python3
"""MeshCator USB maintenance. pip install pyserial. No flash/partition writes."""
import argparse,struct,time,json,os
from pathlib import Path
MAGIC=b'\x7eMC'
def checksum(data):
 n=2166136261
 for b in data:n=((n^b)*16777619)&0xffffffff
 return n
class Client:
 def __init__(self,serial):self.serial=serial
 def rpc(self,op,data=b''):
  payload=MAGIC+bytes([op])+data
  if len(payload)>176:raise ValueError('frame too large')
  self.serial.write(b'<'+struct.pack('<H',len(payload))+payload)
  deadline=time.monotonic()+8
  while time.monotonic()<deadline:
   if self.serial.read(1)!=b'>':continue
   h=self.serial.read(2)
   if len(h)!=2:continue
   n=struct.unpack('<H',h)[0]
   if n>176:continue
   answer=self.serial.read(n)
   if answer[:3]!=MAGIC:continue
   if len(answer)<4 or answer[3]:raise RuntimeError('Device rejected request (2 = incoming data changed during restore): '+answer.hex())
   return answer[4:]
  raise TimeoutError('No maintenance response. Check firmware, USB port and close other serial clients.')
 def get(self,name):
  total=struct.unpack('<I',self.rpc(1,name.encode()))[0]
  if total>4*1024*1024:raise ValueError('resource too large')
  out=bytearray()
  while len(out)<total:
   chunk=self.rpc(2,struct.pack('<I',len(out)))
   if not chunk:raise RuntimeError('short download')
   out.extend(chunk)
  self.rpc(6);return bytes(out)
 def put(self,name,data):
  self.rpc(3,name.encode())
  for at in range(0,len(data),160):self.rpc(4,struct.pack('<I',at)+data[at:at+160])
  self.rpc(5,struct.pack('<I',checksum(data)))
def save(path,data):
 with open(path,'xb') as f:os.chmod(path,0o600);f.write(data)
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--port',required=True)
 sub=p.add_subparsers(dest='cmd',required=True);sub.add_parser('info');h=sub.add_parser('haptics');h.add_argument('--test',action='store_true',help='Play one 750-ms motor test pulse')
 for cmd in ['get','put']:
  a=sub.add_parser(cmd);a.add_argument('resource');a.add_argument('file',type=Path)
 for cmd in ['backup','restore']:
  a=sub.add_parser(cmd);a.add_argument('directory',type=Path)
 a=sub.add_parser('import');a.add_argument('file',type=Path)
 a=sub.add_parser('set');a.add_argument('field');a.add_argument('value')
 args=p.parse_args();import serial
 with serial.Serial(args.port,115200,timeout=1) as port:
  c=Client(port)
  if args.cmd=='info':print(c.rpc(0).decode())
  elif args.cmd=='haptics':print(c.rpc(8 if args.test else 7).decode())
  elif args.cmd=='set':
   fields={'theme':(1,0),'language':(1,1),'setup':(1,3),'frequency':(2,0),'bandwidth':(2,1),'sf':(2,2),'cr':(2,3),'power':(2,4),'hash_bytes':(2,5),'forwarding':(3,0),'gps':(3,1),'share_location':(3,2),'bluetooth':(3,3),'brightness':(3,4),'timeout':(3,5),'map_source':(3,6),'show_contacts':(3,7),'name':(4,None),'bluetooth_name':(5,None),'pin':(6,None),'mute':(-1,1),'alert_mode':(-1,2),'volume':(-1,3),'wake':(-1,4)}
   if args.field not in fields:raise ValueError('Fields: '+', '.join(fields))
   if any(ord(x)<32 for x in args.value):raise ValueError('control character')
   lines=c.get('settings').decode().splitlines();line,col=fields[args.field]
   if line==-1:line=next(i for i,s in enumerate(lines) if s.startswith('N1 '))
   if col is None:lines[line]=args.value
   else:
    parts=lines[line].split();parts[col]=args.value;lines[line]=' '.join(parts)
   if args.field in ['frequency','bandwidth','sf','cr']:parts=lines[1].split();parts[4]='-1';lines[1]=' '.join(parts)
   c.put('settings',('\n'.join(lines)+'\n').encode())
  elif args.cmd=='get':save(args.file,c.get(args.resource))
  elif args.cmd=='put':c.put(args.resource,args.file.read_bytes())
  elif args.cmd=='import':
   for line in args.file.read_text().splitlines():
    if line:c.put('entry',line.encode())
  elif args.cmd=='backup':
   args.directory.mkdir(mode=0o700,parents=True,exist_ok=False)
   for name in ['settings','history','catalog']:save(args.directory/name,c.get(name))
   print('Settings, history and catalog saved. Radio identity and full flash require tools/backup_launcher.py.')
  elif args.cmd=='restore':
   # Catalog imports are additive; never erase unrelated contacts or channels.
   for line in (args.directory/'catalog').read_text().splitlines():
    if line:c.put('entry',line.encode())
   c.put('settings',(args.directory/'settings').read_bytes());c.put('history',(args.directory/'history').read_bytes())
if __name__=='__main__':main()
