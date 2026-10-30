#!/usr/bin/env python3
"""Select verified MeshCator in this pager's launcher without changing app/data partitions."""
import argparse,hashlib,json,struct,sys,zlib
from pathlib import Path
from migrate_launcher import partitions

def main():
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('--port',required=True);p.add_argument('--esptool-dir',required=True);p.add_argument('--firmware',type=Path,required=True);p.add_argument('--table',type=Path,required=True);p.add_argument('--backup-dir',type=Path,required=True)
 a=p.parse_args();sys.path.insert(0,a.esptool_dir)
 from esptool.targets.esp32s3 import ESP32S3ROM
 esp=ESP32S3ROM(a.port,115200);esp.connect();esp=esp.run_stub();esp.flash_set_parameters(0x1000000)
 try:
  table=esp.read_flash(0x8000,4096)
  assert table==a.table.read_bytes().ljust(4096,b'\xff'),'Unexpected partition table'
  entries=partitions(table);assert entries['meshcator'][:4]==(0x6e0000,0x240000,0,0x14)
  slots=sorted(v[3]-0x10 for v in entries.values() if v[2]==0 and 0x10<=v[3]<0x20)
  assert slots==list(range(5)),'Unexpected OTA slots'
  firmware=a.firmware.read_bytes();assert esp.flash_md5sum(0x6e0000,len(firmware)).lower()==hashlib.md5(firmware).hexdigest(),'Firmware mismatch'
  old=esp.read_flash(0xe000,8192)
  backup=a.backup_dir/'ota-before-select-0.3.1.bin'
  with backup.open('xb') as f:backup.chmod(0o600);f.write(old)
  seqs=[]
  for off in (0,4096):
   seq=struct.unpack_from('<I',old,off)[0];crc=struct.unpack_from('<I',old,off+28)[0]
   assert seq!=0xffffffff and crc==zlib.crc32(old[off:off+4],0xffffffff),'Invalid OTA record'
   seqs.append(seq)
  sequence=(max(seqs)//5+1)*5;assert sequence<0xfffffffe
  index=seqs.index(min(seqs));address=0xe000+index*4096
  record=struct.pack('<I',sequence)+b'\xff'*24+struct.pack('<I',zlib.crc32(struct.pack('<I',sequence),0xffffffff))
  data=record.ljust(4096,b'\xff');compressed=zlib.compress(data,9)
  esp.flash_defl_begin(len(data),len(compressed),address)
  for seq,off in enumerate(range(0,len(compressed),esp.FLASH_WRITE_SIZE)):esp.flash_defl_block(compressed[off:off+esp.FLASH_WRITE_SIZE],seq,timeout=30)
  esp.read_reg(esp.CHIP_DETECT_MAGIC_REG_ADDR,timeout=30)
  assert esp.flash_md5sum(address,4096).lower()==hashlib.md5(data).hexdigest()
  other=1-index;assert esp.flash_md5sum(0xe000+other*4096,4096).lower()==hashlib.md5(old[other*4096:(other+1)*4096]).hexdigest()
  (a.backup_dir/'boot-select-0.3.1.json').write_text(json.dumps({'verified':True,'ota_slot':4,'sequence':sequence,'offset':hex(address),'previous_sequences':seqs},indent=2))
  print('MeshCator boot selection verified; previous selector retained.',flush=True)
  esp.flash_begin(0,0);esp.flash_defl_finish(False);esp.hard_reset()
 finally:esp._port.close()
if __name__=='__main__':main()
