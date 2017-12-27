#!/usr/bin/env python
#*******************************************************************************
#*   Ledger Blue
#*   (c) 2016 Ledger
#*
#*  Licensed under the Apache License, Version 2.0 (the "License");
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*      http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an "AS IS" BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#********************************************************************************
from ledgerblue.comm import getDongle
from ledgerblue.commException import CommException
from secp256k1 import PublicKey

bipp44_path = (
               "8000002C"
              +"80000378"
              +"80000000"
              +"00000000"
              +"00000000")

# sending to AHXSMB19pWytwJ7vzvCw5aWmd1DUniDKRT

# NEP-5
#             80020000ffd1014f0400e1f505143775292229eccdf904f16fff8e83e7cffdc0f0ce14175342b16a9ad150e200dc1d2c9d19052013773153c1087472616e736665726711c4d1f4fba619f2628870d36e3a9773e874705b00000000000000000001ceab6ef5c594711114ca99ae8c24afe3b673eb34a43ca50276cab6b04fc84780010002e72d286979ee6cb1b7e65dfddfb2e384100b8d148e7758de42e4168b71792c6001000000000000003177132005199d2c1ddc00e250d19a6ab1425317e72d286979ee6cb1b7e65dfddfb2e384100b8d148e7758de42e4168b71792c60dfee424e00000000175342b16a9ad150e200dc1d2c9d190520137731008000002c80000378
#             800280000c800000000000000000000000
textToSign_01 = bytes(("d1014f0400e1f505143775292229eccdf904f16fff8e83e7cffdc0f0ce14175342b16a9ad150e200dc1d2c9d19052013773153c1087472616e736665726711c4d1f4fba619f2628870d36e3a9773e874705b00000000000000000001ceab6ef5c594711114ca99ae8c24afe3b673eb34a43ca50276cab6b04fc84780010002e72d286979ee6cb1b7e65dfddfb2e384100b8d148e7758de42e4168b71792c6001000000000000003177132005199d2c1ddc00e250d19a6ab1425317e72d286979ee6cb1b7e65dfddfb2e384100b8d148e7758de42e4168b71792c60dfee424e00000000175342b16a9ad150e200dc1d2c9d19052013773100" + bipp44_path + "").decode('hex'))

textToSignArray = [textToSign_01]

dongle = getDongle(True)
publicKey = dongle.exchange(bytes(("80040000FF"+ bipp44_path).decode('hex')))
print "publicKey " + str(publicKey).encode('hex')

for textToSign in textToSignArray:
	try:
		offset = 0
		while offset <> len(textToSign):
			if (len(textToSign) - offset) > 255:
				chunk = textToSign[offset : offset + 255] 
			else:
				chunk = textToSign[offset:]
			if (offset + len(chunk)) == len(textToSign):
				p1 = 0x80
			else:
				p1 = 0x00
			apdu = bytes("8002".decode('hex')) + chr(p1) + chr(0x00) + chr(len(chunk)) + bytes(chunk)
			signature = dongle.exchange(apdu)
			offset += len(chunk)  	
		print "signature " + str(signature).encode('hex')
	except CommException as comm:
		if comm.sw == 0x6985:
			print "Aborted by user"
		else:
			print "Invalid status " + hex(comm.sw)

