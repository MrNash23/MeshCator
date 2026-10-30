import unittest,struct,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from meshcator_usb import Client,checksum,MAGIC
class FakeSerial:
 def __init__(self):self.queue=bytearray();self.data=b'abcd'*101;self.upload=bytearray();self.requests=[]
 def write(self,frame):
  assert frame[0:1]==b'<' and struct.unpack('<H',frame[1:3])[0]==len(frame)-3
  p=frame[3:];assert p[:3]==MAGIC and len(p)<=176
  op=p[3];data=p[4:];self.requests.append(op);out=b''
  if op==1:out=struct.pack('<I',len(self.data))
  elif op==2:at=struct.unpack('<I',data)[0];out=self.data[at:at+160]
  elif op==4:
   at=struct.unpack('<I',data[:4])[0];assert at==len(self.upload);self.upload+=data[4:]
  elif op==5:assert struct.unpack('<I',data)[0]==checksum(self.upload)
  answer=MAGIC+b'\0'+out;self.queue+=b'>'+struct.pack('<H',len(answer))+answer
 def read(self,n):out=self.queue[:n];del self.queue[:n];return bytes(out)
class Tests(unittest.TestCase):
 def test_chunks_and_checksum(self):
  f=FakeSerial();c=Client(f);self.assertEqual(c.get('history'),f.data);c.put('history',f.data);self.assertEqual(f.upload,f.data);self.assertEqual(f.requests.count(4),3)
 def test_oversized_frame(self):
  with self.assertRaises(ValueError):Client(FakeSerial()).rpc(1,b'x'*180)
if __name__=='__main__':unittest.main()
