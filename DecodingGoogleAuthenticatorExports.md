# Decoding Google Authenticator Exports for Ente

## Overview

Google Authenticator exports OTP secrets using a proprietary `otpauth-migration://` URI format. These exports:

* Contain one or more OTP entries
* Are base64-encoded
* Embed a protobuf payload

Ente Auth requires manual entry in the form:

* Issuer
* Secret (Base32)
* Account
* Notes (optional)

This guide provides a CLI tool to decode Google exports into a format suitable for Ente.

---

## Features

* Decodes `otpauth-migration://` URIs
* Extracts multiple OTP entries per export
* Converts secrets to Base32 (compatible with authenticator apps)
* Outputs:

  * Human-readable CLI format
  * CSV for bulk handling

---

## Requirements

* Fedora (or any Linux)
* Python 3.8+
* No external dependencies

---

## Installation

Save the script as:

```bash
google_migration_to_ente.py
```

Make it executable:

```bash
chmod +x google_migration_to_ente.py
```

---

## Usage

### 1. From file

Create a file (e.g. `exports.txt`) with one export per line:

```
otpauth-migration://offline?data=...
otpauth-migration://offline?data=...
```

Run:

```bash
./google_migration_to_ente.py exports.txt
```

---

### 2. Output CSV

```bash
./google_migration_to_ente.py exports.txt -o ente_import.csv
```

---

### 3. Pipe input

```bash
cat <<'EOF' | ./google_migration_to_ente.py -o ente_import.csv
otpauth-migration://offline?data=...
EOF
```

---

## Output Format

Each OTP entry is printed as:

```
[1]
  Issuer : Example
  Secret : JBSWY3DPEHPK3PXP
  Account: user@example.com
  Notes  : type=TOTP; digits=6; algorithm=SHA1
```

### Field Mapping to Ente

| Script Output | Ente Field |
| ------------- | ---------- |
| Issuer        | Issuer     |
| Secret        | Secret     |
| Account       | Account    |
| Notes         | Notes      |

---

## Script

```python
#!/usr/bin/env python3

import argparse
import base64
import csv
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import List
from urllib.parse import urlparse, parse_qs

ALGORITHM_MAP = {0:"ALGORITHM_UNSPECIFIED",1:"SHA1",2:"SHA256",3:"SHA512",4:"MD5"}
DIGITS_MAP = {0:"DIGIT_COUNT_UNSPECIFIED",1:"6",2:"8"}
OTP_TYPE_MAP = {0:"OTP_TYPE_UNSPECIFIED",1:"HOTP",2:"TOTP"}

@dataclass
class OtpEntry:
    issuer: str
    secret_base32: str
    account: str
    notes: str

class ProtoReader:
    def __init__(self,data:bytes):
        self.data=data; self.pos=0
    def eof(self): return self.pos>=len(self.data)
    def read_byte(self): b=self.data[self.pos]; self.pos+=1; return b
    def read_varint(self):
        shift=0; result=0
        while True:
            b=self.read_byte()
            result|=(b&0x7F)<<shift
            if not(b&0x80): return result
            shift+=7
    def read_length_delimited(self):
        length=self.read_varint()
        out=self.data[self.pos:self.pos+length]
        self.pos+=length
        return out
    def skip_field(self,wt):
        if wt==0: self.read_varint()
        elif wt==1: self.pos+=8
        elif wt==2: self.pos+=self.read_varint()
        elif wt==5: self.pos+=4


def decode_base64_url_nopad(s):
    return base64.urlsafe_b64decode(s + "="*((4-len(s)%4)%4))


def parse_otp_parameters(data):
    r=ProtoReader(data)
    out={"secret":b"","name":"","issuer":"","algorithm":0,"digits":0,"type":0,"counter":None}
    while not r.eof():
        tag=r.read_varint(); fn=tag>>3; wt=tag&7
        if fn==1 and wt==2: out["secret"]=r.read_length_delimited()
        elif fn==2 and wt==2: out["name"]=r.read_length_delimited().decode()
        elif fn==3 and wt==2: out["issuer"]=r.read_length_delimited().decode()
        elif fn==4 and wt==0: out["algorithm"]=r.read_varint()
        elif fn==5 and wt==0: out["digits"]=r.read_varint()
        elif fn==6 and wt==0: out["type"]=r.read_varint()
        elif fn==7 and wt==0: out["counter"]=r.read_varint()
        else: r.skip_field(wt)
    return out


def parse_payload(data):
    r=ProtoReader(data); entries=[]
    while not r.eof():
        tag=r.read_varint(); fn=tag>>3; wt=tag&7
        if fn==1 and wt==2:
            entries.append(parse_otp_parameters(r.read_length_delimited()))
        else: r.skip_field(wt)
    return entries


def to_base32(b):
    return base64.b32encode(b).decode().rstrip("=")


def make_notes(e):
    parts=[]
    if e["type"]: parts.append(f"type={OTP_TYPE_MAP.get(e['type'])}")
    if e["digits"]: parts.append(f"digits={DIGITS_MAP.get(e['digits'])}")
    if e["algorithm"]: parts.append(f"algorithm={ALGORITHM_MAP.get(e['algorithm'])}")
    if e["counter"] is not None: parts.append(f"counter={e['counter']}")
    return "; ".join(parts)


def extract(uri):
    parsed=urlparse(uri)
    data=parse_qs(parsed.query)["data"][0]
    raw=decode_base64_url_nopad(data)
    entries=parse_payload(raw)
    out=[]
    for e in entries:
        secret=to_base32(e["secret"])
        issuer=e["issuer"]
        account=e["name"]
        if not issuer and ":" in account:
            issuer=account.split(":",1)[0]
        out.append(OtpEntry(issuer,secret,account,make_notes(e)))
    return out


def main():
    p=argparse.ArgumentParser()
    p.add_argument("input",nargs="?")
    p.add_argument("-o","--output")
    a=p.parse_args()

    text=Path(a.input).read_text() if a.input else sys.stdin.read()
    entries=[]
    for line in text.splitlines():
        if line.strip(): entries+=extract(line.strip())

    for i,e in enumerate(entries,1):
        print(f"[{i}]\n  Issuer : {e.issuer}\n  Secret : {e.secret_base32}\n  Account: {e.account}\n  Notes  : {e.notes}\n")

    if a.output:
        with open(a.output,"w",newline="") as f:
            w=csv.writer(f)
            w.writerow(["issuer","secret","account","notes"])
            for e in entries: w.writerow([e.issuer,e.secret_base32,e.account,e.notes])

if __name__=="__main__": main()
```

---

## Notes

* A single export can contain multiple OTP entries
* Most entries are `TOTP`, `6 digits`, `SHA1`
* HOTP entries include a counter (stored in Notes)
* Secrets are converted to Base32 for compatibility

---

## Limitations

* No direct Ente import API used (manual entry still required)
* HOTP counters are not mapped to a dedicated field

---

## License

MIT (or your repository default)
