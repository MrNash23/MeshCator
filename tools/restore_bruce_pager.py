#!/usr/bin/env python3
"""Restore the previously verified isolated Bruce slot and its private data.
Only replaces Roami/free space; retains CURRENT FT8, MeshOS, MeshCator and data.
Defaults to an offline plan, and requires verified complete before/source backups.
"""
import argparse,hashlib,json,struct,sys,zlib
from pathlib import Path
from migrate_launcher import partitions,image_length
from install_bruce_pager import NEW as TARGET,digest
from restore_roami_pager import TARGET as EXPECTED,verified_backup

def plan(current,source):
 assert partitions(current[0x8000:0x9000])==EXPECTED,'Unexpected current layout'
 assert partitions(source[0x8000:0x9000])==TARGET,'Unexpected Bruce source layout'
 payload=source[0x920000:0xe70000];length=image_length(payload)
 assert length==4209856,'Not the saved Bruce image'
 assert payload[0x65c2b:0x65c32]==b'brucef\0' and payload[0x4afbc:0x4afc0]==b'bnv\0','Bruce storage isolation missing'
 for name in ('app0','ft8pag','meshos','meshcator'):
  off,size,*_=TARGET[name];assert image_length(current[off:off+size])<=size
 # The exact saved Bruce table leaves every retained region at its current address.
 for name in EXPECTED.keys()-{'roami','otadata'}:assert EXPECTED[name]==TARGET[name]
 table=source[0x8000:0x9000]
 seqs=[]
 for off in (0xe000,0xf000):
  d=current[off:off+32];seq=struct.unpack_from('<I',d)[0]
  assert seq!=0xffffffff and struct.unpack_from('<I',d,28)[0]==zlib.crc32(d[:4],0xffffffff)
  seqs.append(seq)
 base=(max(seqs)//4+1)*4;assert base+2<0xfffffffe
 def record(seq):
  d=struct.pack('<I',seq);return (d+b'\xff'*24+struct.pack('<I',zlib.crc32(d,0xffffffff))).ljust(4096,b'\xff')
 return payload,table,record(base)+record(base+2)

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for key in ('backup','source','output'):p.add_argument('--'+key,type=Path,required=True)
 p.add_argument('--port');p.add_argument('--esptool-dir');p.add_argument('--apply',action='store_true');a=p.parse_args()
 report_path=a.output/'restoration-report.json'
 if report_path.exists() and json.loads(report_path.read_text()).get('completed'):raise ValueError('Already completed; keep the audit report')
 current=verified_backup(a.backup);source=verified_backup(a.source);payload,table,ota=plan(current,source)
 a.output.mkdir(mode=0o700,parents=True,exist_ok=True)
 for name,data in [('partitions.bin',table),('otadata.bin',ota)]:
  path=a.output/name;path.write_bytes(data);path.chmod(0o600)
 report={'completed':False,'backup_sha256':hashlib.sha256(current).hexdigest(),'source_backup_sha256':hashlib.sha256(source).hexdigest(),'bruce_region_sha256':hashlib.sha256(payload).hexdigest(),'old':EXPECTED,'new':TARGET,'preserved':{}}
 report_path.write_text(json.dumps(report,indent=2));print('Plan verified: restore Bruce and private data; retain current apps and shared data.',flush=True)
 if not a.apply:return
 assert a.port and a.esptool_dir
 sys.path.insert(0,a.esptool_dir);from esptool.targets.esp32s3 import ESP32S3ROM
 e=ESP32S3ROM(a.port,115200);e.connect();e=e.run_stub();e.change_baud(921600);e.flash_set_parameters(0x1000000)
 try:
  assert e.flash_md5sum(0,len(current)).lower()==digest(current),'Device changed; no writes performed'
  report['live_backup_verified']=True;report_path.write_text(json.dumps(report,indent=2))
  def write(off,data):
   c=zlib.compress(data,9);e.flash_defl_begin(len(data),len(c),off)
   for n,pos in enumerate(range(0,len(c),e.FLASH_WRITE_SIZE)):e.flash_defl_block(c[pos:pos+e.FLASH_WRITE_SIZE],n,timeout=30)
   e.read_reg(e.CHIP_DETECT_MAGIC_REG_ADDR,timeout=30);assert e.flash_md5sum(off,len(data)).lower()==digest(data)
   print('Verified write',hex(off),len(data),flush=True)
  write(0x920000,payload);write(0x8000,table);write(0xe000,ota)
  for name in ('app0','coredump','nvs','spiffs','meshos','meshcator','ft8pag','mcfs'):
   off,size,*_=EXPECTED[name];d=e.flash_md5sum(off,size).lower();assert d==digest(current[off:off+size]),name+' changed';report['preserved'][name]=d
  for name,off,size in [('bootloader',0,0x8000),('tail',0xf70000,0x90000)]:
   d=e.flash_md5sum(off,size).lower();assert d==digest(current[off:off+size]);report['preserved'][name]=d
  report['completed']=True;report_path.write_text(json.dumps(report,indent=2));print('Bruce restored; preserved regions verified. Starting Bruce.',flush=True)
  e.flash_begin(0,0);e.flash_defl_finish(False);e.hard_reset()
 finally:report_path.write_text(json.dumps(report,indent=2));e._port.close()
if __name__=='__main__':main()
