import socket
import time
import struct
import datetime

def print_hex(bytes):
    l = [hex(int(i)) for i in bytes]
    print(" ".join(l))

SEVER_IP = '10.15.112.24'
PORT = 16000

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind((SEVER_IP, PORT))
server.listen()

send_data = bytearray(b'\xfe\x00\x13\x0f\x5a\xe1\x85\x45\x02\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff')
#心跳响应包，用于响应0xaa的数据包，返回当前的时间戳
ack = bytearray(b'\xfe\x00\x14\xaa\x5a\xec\x2b\x59\00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff')
#这个用于修改传感器为快速模式的
fast_mode = bytearray(b'\xfe\x00\x13\x0f\x5a\xe1\x85\x45\01\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff')
conn, addr = server.accept()

beat_time = time.time()
time_now = time.time()
time_now = int(time.time())
ret = struct.pack('>I', time_now)
ack[4] = ret[0]
ack[5] = ret[1]
ack[6] = ret[2]
ack[7] = ret[3]
conn.send(ack)

fast_mode[4] = ret[0]
fast_mode[5] = ret[1]
fast_mode[6] = ret[2]
fast_mode[7] = ret[3]
#改变为快速模式
conn.send(fast_mode)

while 1:
    try :
        data = conn.recv(16)
        if data[0] == 0xfe:
            length = int(data[1]) * 256 + int(data[2])
            left_data = conn.recv(length-16)
            data = data + left_data
            # print_hex(data)
            
            if data[3] == 0x10:
                heart = data[16] * 256 + data[17]
                breath = data[18]
                print('heart = %d -- breath = %d -- %s' % (heart, breath, datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S'))) 
            
            if data[3] == 0xaa:
                beat_time = time.time()
                time_now = time.time()
                time_now = int(time.time())
                ret = struct.pack('>I', time_now)
                ack[4] = ret[0]
                ack[5] = ret[1]
                ack[6] = ret[2]
                ack[7] = ret[3]
                conn.send(ack)
                print('send heart ack , bedmode = %d deviceState = %d -- %s' % (data[20], data[18], datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')))
        
    except Exception as e:
        print('ERROR:', e, datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
        conn.close()
        conn, addr = server.accept() 
server.close()
