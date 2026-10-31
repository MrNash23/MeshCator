#!/usr/bin/env python3
"""Device-specific, backup-gated Meshtastic/Roami replacement. Default: prepare only.
Bruce 1.16.1's two DROM partition-label literals are isolated; all code is unchanged.
The upstream image digest, exact literal locations, segment checksums, complete
live backup, copied FT8 slot and every retained region are checked before/after.
Never writes a bootloader; never performs erase_flash. Keep backups private.
"""
import argparse,hashlib,json,struct,sys,zlib
from pathlib import Path
from functools import reduce
from operator import xor
from migrate_launcher import partitions,image_length
UPSTREAM='eb7ecdd8216ca0c54e9722c1fcb386e326b387152fa9cad3469246cb95cb5a39'
OLD={'nvs':(0x9000,0x5000,1,2,0),'otadata':(0xe000,0x2000,1,0,0),'app0':(0x10000,0x180000,0,0x20,0),'coredump':(0x190000,0x10000,1,3,0),'firmwa':(0x1a0000,0x250000,0,0x10,0),'spiffs':(0x3f0000,0x70000,1,0x82,0),'meshos':(0x460000,0x280000,0,0x13,0),'meshcator':(0x6e0000,0x240000,0,0x14,0),'ft8pag':(0x920000,0x250000,0,0x11,0),'roami':(0xb70000,0x300000,0,0x12,0),'mcfs':(0xe70000,0x100000,1,0x82,0)}
NEW={k:v for k,v in OLD.items() if k not in ('firmwa','roami','ft8pag')}
NEW.update(ft8pag=(0x1a0000,0x250000,0,0x10,0),meshos=(0x460000,0x280000,0,0x12,0),meshcator=(0x6e0000,0x240000,0,0x13,0),bruce=(0x920000,0x410000,0,0x11,0),bnv=(0xd30000,0x10000,1,2,0),brucef=(0xd40000,0x130000,1,0x83,0))
def digest(b):return hashlib.md5(b).hexdigest()
def plan(full,merged):
 assert len(full)==0x1000000 and partitions(full[0x8000:0x9000])==OLD,'Wrong pager layout'
 assert hashlib.sha256(merged).hexdigest()==UPSTREAM,'Not the reviewed official release'
 app=bytearray(merged[0x10000:]);assert image_length(app)==len(app)==4209856
 # Read-only DROM segment: these literals are unique suffix-merged strings.
 start=32;address,size=struct.unpack_from('<II',app,24);assert address==0x3c2f0020
 patches=[]
 for offset,old,new in [(0x65c2b,b'spiffs\0',b'brucef\0'),(0x4afbc,b'nvs\0',b'bnv\0')]:
  assert app.count(b'\0'+old if old==b'nvs\0' else old)==1 and app[offset:offset+len(old)]==old
  assert start<=offset and offset+len(old)<=start+size
  app[offset:offset+len(old)]=new;patches.append({'offset':hex(offset),'old':old[:-1].decode(),'new':new[:-1].decode()})
 end=24;check=0xef
 for _ in range(app[1]):
  _,length=struct.unpack_from('<II',app,end);start=end+8;end=start+length;check=reduce(xor,app[start:end],check)
 checksum_end=((end+16)//16)*16;assert app[23]==1 and checksum_end+32==len(app)
 app[checksum_end-1]=check;app[checksum_end:]=hashlib.sha256(app[:checksum_end]).digest();assert image_length(app)==len(app)
 table=b'';last=0x9000
 for name,(off,length,kind,sub,flags) in sorted(NEW.items(),key=lambda kv:kv[1][0]):
  assert off>=last and off+length<=len(full);last=off+length
  if kind==0:assert off%65536==0
  table+=struct.pack('<HBBII16sI',0x50aa,kind,sub,off,length,name.encode(),flags)
 table+=(b'\xeb\xeb'+b'\xff'*14+hashlib.md5(table).digest());table=table.ljust(4096,b'\xff');assert partitions(table)==NEW
 for name in ('app0','meshos','meshcator','ft8pag'):
  off,length,*_=OLD[name];assert image_length(full[off:off+length])<=NEW[name][1]
 assert b'meshtastic' in full[0x1a0000:0x3f0000].lower()
 # Four dense OTA slots. Retain a valid MeshCator fallback; select Bruce newer.
 seqs=[]
 for off in (0xe000,0xf000):
  data=full[off:off+32];seq=struct.unpack_from('<I',data)[0]
  assert seq!=0xffffffff and struct.unpack_from('<I',data,28)[0]==zlib.crc32(data[:4],0xffffffff)
  seqs.append(seq)
 base=(max(seqs)//4+1)*4
 def record(seq):
  first=struct.pack('<I',seq);return (first+b'\xff'*24+struct.pack('<I',zlib.crc32(first,0xffffffff))).ljust(4096,b'\xff')
 ota=record(base)+record(base+2) # (seq-1)%4: MeshCator=3, Bruce=1.
 return bytes(app),table,ota,patches

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--backup',type=Path,required=True);p.add_argument('--upstream',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--port');p.add_argument('--esptool-dir');p.add_argument('--apply',action='store_true');a=p.parse_args()
 existing=a.output/'installation-report.json'
 if existing.exists() and json.loads(existing.read_text()).get('completed'):raise ValueError('Installation already completed; keep its audit report')
 full=a.backup.read_bytes();app,table,ota,patches=plan(full,a.upstream.read_bytes());a.output.mkdir(mode=0o700,parents=True,exist_ok=True)
 for name,data in [('bruce-isolated-app.bin',app),('partitions.bin',table),('otadata.bin',ota)]:
  path=a.output/name;path.write_bytes(data);path.chmod(0o600)
 report={'completed':False,'backup_sha256':hashlib.sha256(full).hexdigest(),'upstream_sha256':UPSTREAM,'app_sha256':hashlib.sha256(app).hexdigest(),'patches':patches,'old':OLD,'new':NEW,'preserved':{}}
 report_path=a.output/'installation-report.json'
 report_path.write_text(json.dumps(report,indent=2))
 print('Plan validated. FT8 -> old Meshtastic slot; Bruce + isolated NVS/LittleFS -> FT8/Roami slots.',flush=True)
 if not a.apply:return
 assert a.port and a.esptool_dir
 sys.path.insert(0,a.esptool_dir);from esptool.targets.esp32s3 import ESP32S3ROM
 esp=ESP32S3ROM(a.port,115200);esp.connect();esp=esp.run_stub();esp.change_baud(921600);esp.flash_set_parameters(0x1000000)
 try:
  assert esp.flash_md5sum(0,len(full)).lower()==digest(full),'Live device changed; no writes'
  report['full_backup_verified']=True;report_path.write_text(json.dumps(report,indent=2))
  def write(off,data):
   compressed=zlib.compress(data,9);esp.flash_defl_begin(len(data),len(compressed),off)
   for n,pos in enumerate(range(0,len(compressed),esp.FLASH_WRITE_SIZE)):esp.flash_defl_block(compressed[pos:pos+esp.FLASH_WRITE_SIZE],n,timeout=30)
   esp.read_reg(esp.CHIP_DETECT_MAGIC_REG_ADDR,timeout=30);assert esp.flash_md5sum(off,len(data)).lower()==digest(data)
   print('Verified write',hex(off),len(data),flush=True)
  # Preserve the entire FT8 slot, including padding, before its old slot is reused.
  write(0x1a0000,full[0x920000:0xb70000])
  write(0x920000,app.ljust(0x410000,b'\xff'))
  write(0xd30000,b'\xff'*0x140000)
  # Commit table and selector only once all images/data have verified.
  write(0x8000,table);write(0xe000,ota)
  for name in ('app0','coredump','nvs','spiffs','meshos','meshcator','ft8pag','mcfs'):
   oldoff,length,*_=OLD[name];off=NEW[name][0];expected=digest(full[oldoff:oldoff+length]);actual=esp.flash_md5sum(off,length).lower();assert actual==expected,name+' modified';report['preserved'][name]=actual
  for name,off,length in [('bootloader',0,0x8000),('tail',0xf70000,0x90000)]:
   actual=esp.flash_md5sum(off,length).lower();assert actual==digest(full[off:off+length]);report['preserved'][name]=actual
  report['completed']=True;report_path.write_text(json.dumps(report,indent=2));print('All retained regions verified. Starting Bruce.',flush=True)
  esp.flash_begin(0,0);esp.flash_defl_finish(False);esp.hard_reset()
 finally:report_path.write_text(json.dumps(report,indent=2));esp._port.close()
if __name__=='__main__':main()
