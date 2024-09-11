"""
    Copyright (C) 2017 - 2021  Zhengyu Peng, https://zpeng.me

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    ----------

    `                      `
    -:.                  -#:
    -//:.              -###:
    -////:.          -#####:
    -/:.://:.      -###++##:
    ..   `://:-  -###+. :##:
           `:/+####+.   :##:
    .::::::::/+###.     :##:
    .////-----+##:    `:###:
     `-//:.   :##:  `:###/.
       `-//:. :##:`:###/.
         `-//:+######/.
           `-/+####/.
             `+##+.
              :##:
              :##:
              :##:
              :##:
              :##:
               .+:

"""

from PySide6.QtCore import QObject, Signal, Slot
import socket


class TCPClient(QObject):
    status = Signal(int, object)
    message = Signal(object, object)
    ERROR = -1
    LISTEN = 1
    CONNECTED = 2
    STOP = 3

    SIG_NORMAL = 0
    SIG_STOP = 1
    SIG_DISCONNECT = 2

    def __init__(self, ip, port):
        QObject.__init__(self)

        self.ip = ip
        self.port = port
        self.tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.tcp_socket.settimeout(1)

        self.signal = self.SIG_NORMAL

    @Slot()
    def start(self):
        try:
            self.tcp_socket.connect((self.ip, self.port))
        except OSError as err:
            print(err)
            # self.status.emit(self.STOP, '')
        else:
            # print('connected')
            self.status.emit(self.CONNECTED, self.ip)

            while True:
                if self.signal == self.SIG_NORMAL:
                    try:

                        data = self.tcp_socket.recv(4096)
                    except socket.timeout as t_out:
                        pass
                    else:
                        if data:
                            self.message.emit(
                                self.ip+':'+str(self.port),
                                data.decode())
                        else:
                            break
                elif self.signal == self.SIG_DISCONNECT:
                    self.signal = self.SIG_NORMAL
                    self.tcp_socket.close()
                    break
        finally:
            self.status.emit(self.STOP, '')

    def send(self, msg):
        self.tcp_socket.sendall(msg.encode())

    def close(self):
        self.signal = self.SIG_DISCONNECT
