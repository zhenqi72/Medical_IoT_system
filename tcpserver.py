# -*- coding: utf-8 -*-
"""
Created on Sat Apr  9 23:14:31 2022

@author: wzq
"""

from socketserver import BaseRequestHandler, TCPServer

class EchoHandler(BaseRequestHandler):
    def handle(self):
        print('Got connection from', self.client_address)
        while True:

            msg = self.request.recv(1337)
            if not msg:
                break
            self.request.send(msg)

if __name__ == '__main__':
    serv = TCPServer(('', 1122), EchoHandler)
    serv.serve_forever()