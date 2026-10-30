#!/usr/bin/env python3
"""Back up and update only the existing MeshCator app slot; never alter partition/OTA/data sectors."""
import argparse,hashlib,json,sys,zlib
from pathlib import Path
from migrate_launcher import partitions,image_length

def main():
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('--port',required=True);p.add_argument('--esptool-dir',required=True);p.add_argument('--firmware',type=Path,required=True);p.add_argument('--table',type=Path,required=True);p.add_argument('--backup-dir',type=Path,required=True)
 p.add_argument('--reuse-slot',type=Path)
 a=p.parse_args();firmware=a.firmware.read_bytes();assert image_length(firmware)==len(firmware)
 a.backup_dir.mkdir(mode=0o700,parents=True,exist_ok=False)
 sys.path.insert(0,a.esptool_dir)
 from esptool.targets.esp32s3 import ESP32S3ROM
 esp=ESP32S3ROM(a.port,115200);esp.connect();esp=esp.run_stub();esp.change_baud(921600);esp.flash_set_parameters(0x1000000)
 report={'completed':False,'backup_verified':False,'before':{},'after':{}}
 try:
  table=esp.read_flash(0x8000,4096);assert table==a.table.read_bytes().ljust(4096,b'\xff'),'Unexpected table'
  entries=partitions(table);address,size,kind,subtype,_=entries['meshcator'];assert (address,size,kind,subtype)==(0x6e0000,0x240000,0,0x14);assert len(firmware)<=size
  backup=a.reuse_slot.read_bytes() if a.reuse_slot else b''
  if len(backup)==size and esp.flash_md5sum(address,size).lower()==hashlib.md5(backup).hexdigest():
   print('Existing complete slot image matches live flash; reusing verified backup.',flush=True)
  else:
   chunks=[]
   for offset in range(0,size,0x10000):
    block=esp.read_flash(address+offset,min(0x10000,size-offset));assert esp.flash_md5sum(address+offset,len(block)).lower()==hashlib.md5(block).hexdigest();chunks.append(block)
    print(f'App backup verified: {offset+len(block)}/{size}',flush=True)
   backup=b''.join(chunks)
  assert esp.flash_md5sum(address,size).lower()==hashlib.md5(backup).hexdigest()
  target=a.backup_dir/'previous-app-slot.bin';target.write_bytes(backup);target.chmod(0o600)
  report.update(backup_verified=True,backup_sha256=hashlib.sha256(backup).hexdigest(),firmware_sha256=hashlib.sha256(firmware).hexdigest())
  preserved={'header':(0,0x9000)}
  preserved.update({k:v[:2] for k,v in entries.items() if k!='meshcator'})
  for name,(offset,length) in preserved.items():report['before'][name]=esp.flash_md5sum(offset,length).lower()
  (a.backup_dir/'update-report.json').write_text(json.dumps(report,indent=2))
  print('Full app slot backed up and verified. Writing firmware only.',flush=True)
  compressed=zlib.compress(firmware,9);esp.flash_defl_begin(len(firmware),len(compressed),address)
  for seq,offset in enumerate(range(0,len(compressed),esp.FLASH_WRITE_SIZE)):esp.flash_defl_block(compressed[offset:offset+esp.FLASH_WRITE_SIZE],seq,timeout=30)
  esp.read_reg(esp.CHIP_DETECT_MAGIC_REG_ADDR,timeout=30)
  assert esp.flash_md5sum(address,len(firmware)).lower()==hashlib.md5(firmware).hexdigest(),'Firmware readback failed'
  for name,(offset,length) in preserved.items():
   digest=esp.flash_md5sum(offset,length).lower();assert digest==report['before'][name],name+' changed';report['after'][name]=digest
  report['completed']=True;print('Firmware and all preserved regions verified. Restarting.',flush=True)
  esp.flash_begin(0,0);esp.flash_defl_finish(False);esp.hard_reset()
 finally:
  (a.backup_dir/'update-report.json').write_text(json.dumps(report,indent=2));esp._port.close()
if __name__=='__main__':main()
