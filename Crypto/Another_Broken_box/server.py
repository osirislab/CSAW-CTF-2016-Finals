import SocketServer
from functools import wraps
import random, time, os, errno, signal

FLAG = 'flag{br0k3n_b0x_f0r3v3r}'
assert len(FLAG) == 24
key = "b33f74c0"

sbox = [
    0x26, 0x73, 0xC0, 0x0D, 0x5A, 0xA7, 0xF4, 0x41, 0x8E, 0xDB, 0x28, 0x75, 0xC2, 0x0F, 0x5C, 0xA9,
    0xF6, 0x43, 0x90, 0xDD, 0x2A, 0x77, 0xC4, 0x11, 0x5E, 0xAB, 0xF8, 0x45, 0x92, 0xDF, 0x2C, 0x79,
    0xC6, 0x13, 0x60, 0xAD, 0xFA, 0x47, 0x94, 0xE1, 0x2E, 0x7B, 0xC8, 0x15, 0x62, 0xAF, 0xFC, 0x49,
    0x96, 0xE3, 0x30, 0x7D, 0xCA, 0x17, 0x64, 0xB1, 0xFE, 0x4B, 0x98, 0xE5, 0x32, 0x7F, 0xCC, 0x19,
    0x66, 0xB3, 0x00, 0x4D, 0x9A, 0xE7, 0x34, 0x81, 0xCE, 0x1B, 0x68, 0xB5, 0x02, 0x4F, 0x9C, 0xE9,
    0x36, 0x83, 0xD0, 0x1D, 0x6A, 0xB7, 0x04, 0x51, 0x9E, 0xEB, 0x38, 0x85, 0xD2, 0x1F, 0x6C, 0xB9,
    0x06, 0x53, 0xA0, 0xED, 0x3A, 0x87, 0xD4, 0x21, 0x6E, 0xBB, 0x08, 0x55, 0xA2, 0xEF, 0x3C, 0x89,
    0xD6, 0x23, 0x70, 0xBD, 0x0A, 0x57, 0xA4, 0xF1, 0x3E, 0x8B, 0xD8, 0x25, 0x72, 0xBF, 0x0C, 0x59,
    0xA6, 0xF3, 0x40, 0x8D, 0xDA, 0x27, 0x74, 0xC1, 0x0E, 0x5B, 0xA8, 0xF5, 0x42, 0x8F, 0xDC, 0x29,
    0x76, 0xC3, 0x10, 0x5D, 0xAA, 0xF7, 0x44, 0x91, 0xDE, 0x2B, 0x78, 0xC5, 0x12, 0x5F, 0xAC, 0xF9,
    0x46, 0x93, 0xE0, 0x2D, 0x7A, 0xC7, 0x14, 0x61, 0xAE, 0xFB, 0x48, 0x95, 0xE2, 0x2F, 0x7C, 0xC9,
    0x16, 0x63, 0xB0, 0xFD, 0x4A, 0x97, 0xE4, 0x31, 0x7E, 0xCB, 0x18, 0x65, 0xB2, 0xFF, 0x4C, 0x99,
    0xE6, 0x33, 0x80, 0xCD, 0x1A, 0x67, 0xB4, 0x01, 0x4E, 0x9B, 0xE8, 0x35, 0x82, 0xCF, 0x1C, 0x69,
    0xB6, 0x03, 0x50, 0x9D, 0xEA, 0x37, 0x84, 0xD1, 0x1E, 0x6B, 0xB8, 0x05, 0x52, 0x9F, 0xEC, 0x39,
    0x86, 0xD3, 0x20, 0x6D, 0xBA, 0x07, 0x54, 0xA1, 0xEE, 0x3B, 0x88, 0xD5, 0x22, 0x6F, 0xBC, 0x09,
    0x56, 0xA3, 0xF0, 0x3D, 0x8A, 0xD7, 0x24, 0x71, 0xBE, 0x0B, 0x58, 0xA5, 0xF2, 0x3F, 0x8C, 0xD9
]

sinv = [
    0x42, 0xC7, 0x4C, 0xD1, 0x56, 0xDB, 0x60, 0xE5, 0x6A, 0xEF, 0x74, 0xF9, 0x7E, 0x03, 0x88, 0x0D,
    0x92, 0x17, 0x9C, 0x21, 0xA6, 0x2B, 0xB0, 0x35, 0xBA, 0x3F, 0xC4, 0x49, 0xCE, 0x53, 0xD8, 0x5D,
    0xE2, 0x67, 0xEC, 0x71, 0xF6, 0x7B, 0x00, 0x85, 0x0A, 0x8F, 0x14, 0x99, 0x1E, 0xA3, 0x28, 0xAD,
    0x32, 0xB7, 0x3C, 0xC1, 0x46, 0xCB, 0x50, 0xD5, 0x5A, 0xDF, 0x64, 0xE9, 0x6E, 0xF3, 0x78, 0xFD,
    0x82, 0x07, 0x8C, 0x11, 0x96, 0x1B, 0xA0, 0x25, 0xAA, 0x2F, 0xB4, 0x39, 0xBE, 0x43, 0xC8, 0x4D,
    0xD2, 0x57, 0xDC, 0x61, 0xE6, 0x6B, 0xF0, 0x75, 0xFA, 0x7F, 0x04, 0x89, 0x0E, 0x93, 0x18, 0x9D,
    0x22, 0xA7, 0x2C, 0xB1, 0x36, 0xBB, 0x40, 0xC5, 0x4A, 0xCF, 0x54, 0xD9, 0x5E, 0xE3, 0x68, 0xED,
    0x72, 0xF7, 0x7C, 0x01, 0x86, 0x0B, 0x90, 0x15, 0x9A, 0x1F, 0xA4, 0x29, 0xAE, 0x33, 0xB8, 0x3D,
    0xC2, 0x47, 0xCC, 0x51, 0xD6, 0x5B, 0xE0, 0x65, 0xEA, 0x6F, 0xF4, 0x79, 0xFE, 0x83, 0x08, 0x8D,
    0x12, 0x97, 0x1C, 0xA1, 0x26, 0xAB, 0x30, 0xB5, 0x3A, 0xBF, 0x44, 0xC9, 0x4E, 0xD3, 0x58, 0xDD,
    0x62, 0xE7, 0x6C, 0xF1, 0x76, 0xFB, 0x80, 0x05, 0x8A, 0x0F, 0x94, 0x19, 0x9E, 0x23, 0xA8, 0x2D,
    0xB2, 0x37, 0xBC, 0x41, 0xC6, 0x4B, 0xD0, 0x55, 0xDA, 0x5F, 0xE4, 0x69, 0xEE, 0x73, 0xF8, 0x7D,
    0x02, 0x87, 0x0C, 0x91, 0x16, 0x9B, 0x20, 0xA5, 0x2A, 0xAF, 0x34, 0xB9, 0x3E, 0xC3, 0x48, 0xCD,
    0x52, 0xD7, 0x5C, 0xE1, 0x66, 0xEB, 0x70, 0xF5, 0x7A, 0xFF, 0x84, 0x09, 0x8E, 0x13, 0x98, 0x1D,
    0xA2, 0x27, 0xAC, 0x31, 0xB6, 0x3B, 0xC0, 0x45, 0xCA, 0x4F, 0xD4, 0x59, 0xDE, 0x63, 0xE8, 0x6D,
    0xF2, 0x77, 0xFC, 0x81, 0x06, 0x8B, 0x10, 0x95, 0x1A, 0x9F, 0x24, 0xA9, 0x2E, 0xB3, 0x38, 0xBD
]

def T(byte, n):
    return (byte >> n) | ((byte & ((1 << n) - 1)) << (8 - n))

def B_enc(block, key):
    B = [ord(b) for b in block]
    K = [ord(b) for b in key]

    for i in xrange(8):
        B = [ sbox[B[j] ^ K[(i + j) & 7]] for j in xrange(8) ]
        B = [ T(B[j], j) for j in xrange(8) ]
        B = [ B[j] ^ B[i] if i != j else B[j] for j in xrange(8) ]
    return ''.join(chr(b) for b in B)

def B_dec(block, key):
    B = [ord(b) for b in block]
    K = [ord(b) for b in key]

    for i in xrange(7, -1, -1):
        B = [ B[j] ^ B[i] if i != j else B[j] for j in xrange(8) ]
        B = [ T(B[j], 8 - j) for j in xrange(8) ]
        B = [ sinv[B[j]] ^ K[(i + j) & 7] for j in xrange(8) ]
    return ''.join(chr(b) for b in B)

def encrypt(msg, key):
    c = ""
    for x in xrange(0, len(msg), 8):
        c += B_enc(msg[x:x+8], key)

    return c
    
def decrypt(c, key):
    msg = ""
    for x in xrange(0, len(c), 8):
        msg += B_dec(c[x:x+8], key)

    return msg

class TimeoutError(Exception):
    pass

def timeout(seconds=10, error_message=os.strerror(errno.ETIME)):
    def decorator(func):
        def _handle_timeout(signum, frame):
            raise TimeoutError(error_message)

        def wrapper(*args, **kwargs):
            signal.signal(signal.SIGALRM, _handle_timeout)
            signal.alarm(seconds)
            try:
                result = func(*args, **kwargs)
            finally:
                signal.alarm(0)
            return result

        return wraps(func)(wrapper)

    return decorator

@timeout(10)
def recv(req, bytes):
        return req.recv(bytes)

def serve(req):
    msg = FLAG
    enc = encrypt(msg, key)
    assert decrypt(enc, key) == FLAG
    
    req.sendall('[+]Encrypted flag:' + enc.encode('hex'))

    while True:
        req.sendall('\n[!]Input your message to encrypt:')
        msg = recv(req, 1024)
        msg = msg.rstrip()
        if len(msg) % 8 != 0:
            req.sendall('[-]Wrong length, quitting...')

        enc = encrypt(msg, key)
        req.sendall('[+]Ciphertext:' + enc.encode('hex'))

class incoming(SocketServer.BaseRequestHandler):
    def handle(self):
        req = self.request
        serve(req)

class ReusableTCPServer(SocketServer.ForkingMixIn, SocketServer.TCPServer):
    pass

SocketServer.TCPServer.allow_reuse_address = True
server = ReusableTCPServer(("0.0.0.0", 31338), incoming)
server.timeout = 60
server.serve_forever()
