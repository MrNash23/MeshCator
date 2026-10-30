#!/usr/bin/env python3
"""Device-specific migration; --apply requires a complete, matching flash backup.
Never calls erase_flash or writes bootloader/NVS/filesystems. Keep the full backup
private: it contains identities, contacts, channel secrets and pairing data.
"""
import argparse, hashlib, json, struct, sys, zlib
from functools import reduce
from operator import xor
from pathlib import Path

EXPECTED = {
 'nvs': (0x9000,0x5000), 'otadata': (0xe000,0x2000),
 'app0': (0x10000,0x180000), 'coredump': (0x190000,0x10000),
 'firmwa': (0x1a0000,0x250000), 'spiffs': (0x3f0000,0x70000),
 'meshos': (0x460000,0x280000), 'meshcator': (0x6e0000,0x190000),
 'ft8pag': (0x870000,0x300000), 'roami': (0xb70000,0x300000),
 'mcfs': (0xe70000,0x100000),
}

def partitions(data):
 result={}
 for off in range(0,3072,32):
  entry=data[off:off+32]
  if entry[:2]!=b'\xaaP':
   if entry[:16]!=b'\xeb\xeb'+b'\xff'*14 or entry[16:]!=hashlib.md5(data[:off]).digest():raise ValueError('Invalid partition-table checksum')
   break
  _,kind,subtype,address,size,label,flags=struct.unpack('<HBBII16sI',entry)
  name=label.split(b'\0')[0].decode('ascii')
  if name in result:raise ValueError('Duplicate partition')
  result[name]=(address,size,kind,subtype,flags)
 return result

def image_length(data):
 if len(data)<24 or data[0]!=0xe9:raise ValueError('Invalid app image')
 end=24;checksum=0xef
 for _ in range(data[1]):
  _,size=struct.unpack_from('<II',data,end)
  start=end+8;end=start+size
  if end>len(data):raise ValueError('App segment exceeds partition')
  checksum=reduce(xor,data[start:end],checksum)
 checksum_end=((end+16)//16)*16
 end=checksum_end+(32 if data[23] else 0)
 if end>len(data):raise ValueError('App checksum exceeds partition')
 if data[checksum_end-1]!=checksum:raise ValueError('App segment checksum mismatch')
 if data[23] and hashlib.sha256(data[:checksum_end]).digest()!=data[checksum_end:end]:raise ValueError('App SHA256 mismatch')
 return end

def md5(data):return hashlib.md5(data).hexdigest()

def main():
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('--backup',type=Path,required=True)
 parser.add_argument('--table',type=Path,required=True)
 parser.add_argument('--firmware',type=Path,required=True)
 parser.add_argument('--esptool-dir',type=Path,required=True)
 parser.add_argument('--port',default='/dev/cu.usbmodem11401')
 parser.add_argument('--apply',action='store_true')
 args=parser.parse_args()
 full=args.backup.read_bytes();table=args.table.read_bytes();firmware=args.firmware.read_bytes()
 if len(full)!=0x1000000:raise ValueError('Backup must cover all 16 MB')
 old=partitions(full[0x8000:0x9000]);new=partitions(table)
 if set(old)!=set(EXPECTED) or set(new)!=set(old):raise ValueError('Unexpected partition labels')
 for name,(address,size) in EXPECTED.items():
  if old[name][:2]!=(address,size):raise ValueError('Unexpected original layout: '+name)
  expected=(0x6e0000,0x240000) if name=='meshcator' else (0x920000,0x250000) if name=='ft8pag' else (address,size)
  if new[name][:2]!=expected or new[name][2:]!=old[name][2:]:raise ValueError('Unexpected target layout: '+name)
 if len(table)>4096 or len(firmware)>new['meshcator'][1]:raise ValueError('New data too large')
 image_length(firmware)
 ft8=full[0x870000:0xb70000]
 if image_length(ft8)>0x250000 or any(b!=255 for b in ft8[0x250000:]):raise ValueError('FT8 tail contains data; cannot shrink safely')
 # Preserve the whole retained FT8 region, including any metadata after its image.
 moved=ft8[:0x250000]
 out=args.backup.parent
 report={'backup_sha256':hashlib.sha256(full).hexdigest(),'firmware_sha256':hashlib.sha256(firmware).hexdigest(),
         'old':old,'new':new,'verified_before':{},'verified_after':{},'completed':False}
 print('Validated plan: MeshCator 0x6e0000/0x240000; FT8 0x920000/0x250000; other partitions unchanged.',flush=True)
 if not args.apply:return
 sys.path.insert(0,str(args.esptool_dir))
 from esptool.targets.esp32s3 import ESP32S3ROM
 esp=ESP32S3ROM(args.port,115200);esp.connect();esp=esp.run_stub();esp.flash_set_parameters(0x1000000)
 try:
  # ROM/stub is now stopped; no app can mutate data during validation/migration.
  live_digest=esp.flash_md5sum(0,0x1000000).lower()
  if live_digest!=md5(full):raise ValueError('Live flash differs from full backup; no writes performed')
  report['verified_before']['full_flash']=live_digest
  (out/'migration-report.json').write_text(json.dumps(report,indent=2))
  def write(address,data):
   compressed=zlib.compress(data,9)
   esp.flash_defl_begin(len(data),len(compressed),address)
   block=esp.FLASH_WRITE_SIZE
   for seq,off in enumerate(range(0,len(compressed),block)):
    esp.flash_defl_block(compressed[off:off+block],seq,timeout=30)
   esp.read_reg(esp.CHIP_DETECT_MAGIC_REG_ADDR,timeout=30)
   actual=esp.flash_md5sum(address,len(data)).lower()
   if actual!=md5(data):raise ValueError('Read-back mismatch at '+hex(address))
   print('Verified write',hex(address),len(data),'bytes',flush=True)
  write(0x920000,moved)
  write(0x6e0000,firmware)
  # Partition table is the final commit, after both images have verified.
  write(0x8000,table.ljust(4096,b'\xff'))
  for name,(address,size,kind,subtype,flags) in new.items():
   if name=='meshcator':continue
   old_address,old_size,*_=old[name]
   expected=full[old_address:old_address+size]
   actual=esp.flash_md5sum(address,size).lower()
   if actual!=md5(expected):raise ValueError('Preservation check failed: '+name)
   report['verified_after'][name]=actual
  if esp.flash_md5sum(0,0x8000).lower()!=md5(full[:0x8000]):raise ValueError('Bootloader changed')
  report['completed']=True
  (out/'migration-report.json').write_text(json.dumps(report,indent=2))
  print('All preserved partitions verified. Booting MeshCator.',flush=True)
  esp.flash_begin(0,0)
  esp.flash_defl_finish(False)
  esp.hard_reset()
 finally:
  (out/'migration-report.json').write_text(json.dumps(report,indent=2))
  esp._port.close()

if __name__=='__main__':main()
