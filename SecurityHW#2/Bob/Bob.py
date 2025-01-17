import socket
import rsa

def toPubKey(data) :
    data = data.decode('utf-8').split('/')
    pubKeyn = int(data[0])
    pubKeye = int(data[1])

    pubKey = rsa.PublicKey(pubKeyn, pubKeye)
    return pubKey

host = "127.0.0.1"
port = 10001





while True :
    parent = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
    #TCP 소켓 객체 생성

    parent.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    #소켓객체 종료후 해당 포트 번호 재사용
    parent.bind((host,port))

    parent.listen(10)
    #3단계 연결 설정에 따라 동작하는 TCP 클라이언트10대로부터 연결 요청을 기다린다.(최대 10대 TCP 클라이언트 접속가능)

    (child, address) = parent.accept() #parent process에서 기다리다가 받으면 child process에 socket객체와 address에 넘김

    data = child.recv(65565)

    print("received data: {}({}) bytes from {}".format(data,len(data),address))

    pubKey = toPubKey(data)


    with open('example.txt') as f:
        message = f.read().encode()
        crypto = rsa.encrypt(message, pubKey)
        child.sendto(crypto,address)
        child.close()
    parent.close()



