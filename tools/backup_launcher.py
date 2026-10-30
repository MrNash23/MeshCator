#!/usr/bin/env python3
"""Read-only resumable 16-MB backup. Verifies each reused chunk against live flash.
No firmware or partition writes. Stop the app in the ESP bootloader for a coherent
snapshot. A disconnected read loses at most one 64-KB chunk, not the whole image.
"""
import argparse, hashlib, json, os, sys
from pathlib import Path

def main():
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('--port',default='/dev/cu.usbmodem11401')
 parser.add_argument('--baud',type=int,default=921600)
 parser.add_argument('--esptool-dir',type=Path,required=True)
 parser.add_argument('--output',type=Path,required=True)
 parser.add_argument('--reuse-directory',type=Path)
 args=parser.parse_args()
 sys.path.insert(0,str(args.esptool_dir))
 from esptool.targets.esp32s3 import ESP32S3ROM
 chunks=args.output.parent/(args.output.stem+'-chunks');chunks.mkdir(parents=True,exist_ok=True)
 os.chmod(chunks,0o700)
 report={'complete':False,'chunks':{},'total_bytes':0x1000000}
 manifest=chunks/'manifest.json'
 esp=ESP32S3ROM(args.port,115200);esp.connect();esp=esp.run_stub();esp.change_baud(args.baud);esp.flash_set_parameters(0x1000000)
 block=0x10000;blank=b'\xff'*block;blank_md5=hashlib.md5(blank).hexdigest()
 try:
  for address in range(0,0x1000000,block):
   digest=esp.flash_md5sum(address,block).lower()
   path=chunks/f'{address:06x}.bin';data=None
   candidates=[path]
   if args.reuse_directory:candidates.append(args.reuse_directory/f'block-{address:06x}.bin')
   for candidate in candidates:
    if candidate.exists():
     cached=candidate.read_bytes()
     if len(cached)==block and hashlib.md5(cached).hexdigest()==digest:data=cached;break
   if data is None:data=blank if digest==blank_md5 else esp.read_flash(address,block)
   if len(data)!=block or hashlib.md5(data).hexdigest()!=digest:raise ValueError('Chunk verification failed')
   temp=path.with_suffix('.tmp');temp.write_bytes(data);os.chmod(temp,0o600);temp.replace(path)
   report['chunks'][hex(address)]=digest
   manifest.write_text(json.dumps(report,indent=2))
   print(f'Verified {address+block:#x}/0x1000000 ({(address+block)*100//0x1000000}%)',flush=True)
  image=b''.join((chunks/f'{a:06x}.bin').read_bytes() for a in range(0,0x1000000,block))
  if esp.flash_md5sum(0,0x1000000).lower()!=hashlib.md5(image).hexdigest():raise ValueError('Full snapshot verification failed')
  temp=args.output.with_suffix('.tmp');temp.write_bytes(image);os.chmod(temp,0o600);temp.replace(args.output)
  report.update(complete=True,sha256=hashlib.sha256(image).hexdigest(),md5=hashlib.md5(image).hexdigest())
  manifest.write_text(json.dumps(report,indent=2))
  print('Complete backup verified against live flash. Device remains in bootloader for migration.',flush=True)
 finally:esp._port.close()

if __name__=='__main__':main()
