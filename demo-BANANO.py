#!/usr/bin/env python
# *******************************************************************************
# *   Ledger Blue
# *   (c) 2016 Ledger
# *
# *  Licensed under the Apache License, Version 2.0 (the "License");
# *  you may not use this file except in compliance with the License.
# *  You may obtain a copy of the License at
# *
# *      http://www.apache.org/licenses/LICENSE-2.0
# *
# *  Unless required by applicable law or agreed to in writing, software
# *  distributed under the License is distributed on an "AS IS" BASIS,
# *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# *  See the License for the specific language governing permissions and
# *  limitations under the License.
# ********************************************************************************
from ledgerblue.comm import getDongle
from ledgerblue.commException import CommException
import binascii
import sys

bipp44_path = (
    "8000002C"
    + "800000a5"
    + "80000000"
    + "80000000"
    + "80000000")

# sending to AHXSMB19pWytwJ7vzvCw5aWmd1DUniDKRT

# # blank tx
# textToSign_01 = bytes(bytearray.fromhex(
#     "80028000340000000000000000000000000000000000000000000000000000000000000000" + bipp44_path + ""))

# parent(32), link(32), rep(32), balance(16)
# tx with small amount, wide range
textToSign_01 = bytes(bytearray.fromhex(
    "80028000"
    # length
    + "70"

    # parent
    #           1         2         3         4         5         6
    #  1234567890123456789012345678901234567890123456789012345678901234
    + "AA00000000000000000000000000000000000000000000000000000000AA"

    # link
    + "BB00000000000000000000000000000000000000000000000000000000BB"

    # rep
    + "FF00000000000000000000000000000000000000000000000000000000FF"

    # balance
    #                     1         2         3
    # ban        123456789012345678901234567890
    # dec 1334440654591915542993625911497130241
    #           1         2         3
    #  12345678901234567890123456789012
    + "0000000000010101010101010101010101"

    # bip44 path
    + bipp44_path))

# tx with small amount, 0x6d14 error
# textToSign_01 = bytes(bytearray.fromhex(
#     "800280003400000000000000000000FF060102030405000708090A0B0C0D00000000000000" + bipp44_path + ""))

# # tx with small amount, 0x6d14 error, going from 99 to 100.
# textToSign_01 = bytes(bytearray.fromhex(
#     "800280003400000000000000000000FF016302030405000708090A0B0C0D00000000000000" + bipp44_path + ""))

# textToSign_01 = bytes(bytearray.fromhex(
#   "80028000343D6AD425971738FD0C1B8C006B4885D38EDEF802D2ACB8DCAB30933154F6962B" + bipp44_path + ""))
# textToSign_01 = bytes(bytearray.fromhex(
#   "80028000343D6AD425971738FD0C1B8CAA6B4885D38EDEF802D2ACB8DCAB30933154F6962B" + bipp44_path + ""))
# textToSign_01 = bytes(bytearray.fromhex(
#   "80028000343D6AD425971738FD0C1B8CAA6B4885D38EDEF802D2ACB8DCAB30933154F6962B" + bipp44_path + ""))

textToSignArray = [textToSign_01]

dongle = getDongle(True)

#print ("STARTED privateKey ")
#privateKey = dongle.exchange(bytes(bytearray.fromhex("80050000FF"+ bipp44_path)))
#actualPrivateKey = privateKey.hex().upper()
#expectPrivateKey = "55BCF45FFD718D8939FB798AAD5B5C8C55D3F08E42507D0D8479F50AB648639E"
#print ("ACTUAL privateKey " + actualPrivateKey)
#print ("EXPECT privateKey " + expectPrivateKey)
#print ("MATCH? privateKey " + str(actualPrivateKey == expectPrivateKey))

print("STARTED base10[1]")
# buggy for 2560-2569, 5120-5130, 7680-7690
# for value in range(2559, 2570):
for value in range(0, 2570):
    print("STARTED base10[1] " + str(value))
    valueHex = format(value, '06X')
    valueStr = format(value, '016')
    print("INTERIM base10[1] valueHex " + valueHex)
    print("INTERIM base10[1] valueStr " + valueStr)
    inputBase10 = bytes(bytearray.fromhex(
        "80108000"
        # length
        + "03"
        # data
        + valueHex
    ))
    print("INTERIM base10[1] " + inputBase10.hex().upper())
    outputBase10 = dongle.exchange(inputBase10)
    actualBase10 = outputBase10[0x03:0x13].decode("utf-8")
    expectBase10 = valueStr
    print("ACTUAL base10[1] " + actualBase10)
    print("EXPECT base10[1] " + expectBase10)

    match = "yes" if (actualBase10 == expectBase10) else "no"
    print("MATCH? base10[1] " + match)

    if (match == "no"):
        raise ValueError('expected %s actual %s' %
                         (expectBase10, actualBase10))
print("SUCCESS base10[1]")

# b6 = divided
# a7 = remainder

# 0a09 => 2569

#     0100r9
#     ----
# A | 0A09
#      A
#      -
#      009

#      19r6
#     ---
# A | 100
#      A
#      -
#      60

# 0a0A => 2570
#     0101r0
#     ----
# A | 0A0A
#      A0A
#      ---
#      00A

#      19r7
#     ---
# A | 101
#      A
#      -
#      61
#      5A
#      --
#       7

# HID => 80108000020a0b
# HID <= fff00f30303030303030303030303032353731f00fff
# 2b2b00000002b20a0bb2
# 6b6b00000002b 60a0b b6
# 8b8b00000010b800000000000000000000000000000001b8
# fcfc00000002cf0101cf
# 6b6b00000002b6 0101 b6
# 8b8b00000010b800000000000000000000000000000701b8
# fcfc00000002cf0019cf
# 6b6b00000002b6 0019 b6
# 8b8b00000010b800000000000000000000000000050701b8
# fcfc00000002cf0002cf
# 6b6b00000002b6 0002 b6
# 8b8b00000010b800000000000000000000000002050701b8
# fcfc00000002cf0000cf
# 9000

# HID => 8010800003000a09
# HID <= fff00f30303030303030303030303030303139f00fff
# 2b2b00000003b2000a09b2
# 6b6b00000002b6 0a09 b6
# 9c00000000c9
# ac00000000ca
# bc00000001cb
# cc00000001cc
# cdcd00000002dc 0000 dc
# dddd00000002dd 0001 dd
# dc0000000acd
# ec00000000ce
# fc00000001cf => end of first round.
# 9c00000000c9
# ac00000001ca
# ec00000009ce
# fc00000002cf
# 8b8b00000010b800000000000000000000000000000009b8fbfb00000002bf0001bffcfc00000001cf01cf6b6b00000001b601b69c00000000c9ac00000000caec00000001cefc00000001cf8b8b00000010b800000000000000000000000000000109b8fbfb00000001bf00bffcfc00000000cfcf9000

# 2b2b00000003b200000ab2
# 6b6b00000001b60ab6
# 9c00000000c9
# ac00000000ca
# bc00000001cb
# cc00000001cc6d25

sys.exit(1)

print("STARTED publicKey[1] ")
publicKey = dongle.exchange(
    bytes(bytearray.fromhex("80040000FF" + bipp44_path)))
actualPublicKey = publicKey.hex().upper()
expectPublicKey = "0AD1D5CB2DB69BFE31560ED1909135D6D5FA6A471D35FD9B1424C84CA68B9C40"
print("ACTUAL publicKey[1] " + actualPublicKey)
print("EXPECT publicKey[1] " + expectPublicKey)
print("MATCH? publicKey[1] " + str(actualPublicKey == expectPublicKey))

print("STARTED publicKey[2] ")
publicKey = dongle.exchange(
    bytes(bytearray.fromhex("80040000FF" + bipp44_path)))
actualPublicKey = publicKey.hex().upper()
expectPublicKey = "0AD1D5CB2DB69BFE31560ED1909135D6D5FA6A471D35FD9B1424C84CA68B9C40"
print("ACTUAL publicKey[2] " + actualPublicKey)
print("EXPECT publicKey[2] " + expectPublicKey)
print("MATCH? publicKey[2] " + str(actualPublicKey == expectPublicKey))

# print ("STARTED sending transactions (sign on device)")
#
for textToSign in textToSignArray:
    try:
        offset = 0
        while offset != len(textToSign):
            if (len(textToSign) - offset) > 255:
                chunk = textToSign[offset: offset + 255]
            else:
                chunk = textToSign[offset:]
            if (offset + len(chunk)) == len(textToSign):
                p1 = 0x80
            else:
                p1 = 0x00
            apdu = bytes(bytearray.fromhex("8002")) + p1.to_bytes(1, byteorder='big') \
                + 0x00.to_bytes(1, byteorder='big') + \
                len(chunk).to_bytes(1, byteorder='big') + bytes(chunk)
            signature = dongle.exchange(apdu)
            offset += len(chunk)
        print("signature " + signature.hex().upper())
    except CommException as comm:
        if comm.sw == 0x6985:
            print("Aborted by user")
        else:
            print("Invalid status " + hex(comm.sw))

print("SUCCESS sending transactions ")
