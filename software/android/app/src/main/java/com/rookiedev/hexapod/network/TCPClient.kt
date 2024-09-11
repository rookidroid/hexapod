package com.rookiedev.hexapod.network

import java.io.*
import java.net.*


class TCPClient(
    ip: String?,
    port: Int,
    messageListener: OnMessageReceived?,
    onConnect: OnConnectEstablished?,
    onDisconnect: OnDisconnected?
) :
    Thread() {
    private var tcpSocket: Socket? = null
    private var serverIp: InetAddress? = null
    private var serverPort = 0
    private var serverAddress: InetSocketAddress? = null
    private var tcpOut: PrintWriter? = null
    private var tcpIn: BufferedReader? = null
    private var mMessageListener: OnMessageReceived? = null
    private var onConnected: OnConnectEstablished? = null
    private var onDisconnected: OnDisconnected? = null
    private var isConnected = false

    override fun run() {
        try {
            this.tcpSocket = Socket()
            this.tcpSocket!!.soTimeout = 3000
            this.tcpSocket!!.connect(serverAddress, 3000) // connecting socket and set timeout in 3s
            onConnected!!.onConnected()
            tcpOut = PrintWriter(
                BufferedWriter(OutputStreamWriter(this.tcpSocket!!.getOutputStream())),
                true
            )
            tcpIn = BufferedReader(InputStreamReader(this.tcpSocket!!.getInputStream()))
            isConnected = true
            while (isConnected) {
                sleep(1000)
            }
        } catch (e: InterruptedException) {
//            controller.cancelProgressDialog(java.lang.ModuleLayer.Controller.SERVERALERT)
            println(e)
//            controller.alertDialog(0)
//            onDisconnected!!.onDisconnected()
        } catch (e: SocketTimeoutException) {
            println(e)
            onDisconnected!!.onDisconnected()
        } catch (e: ConnectException) {
            println(e)
            onDisconnected!!.onDisconnected()
        }
    }

    /**
     * Sends the message entered by client to the server
     *
     * @param message text entered by client
     */
    fun sendMessage(message: String?) {
//        newMessage = message
//        isNewData = true

        if (this.tcpOut != null && !this.tcpOut!!.checkError()) {
            println("send message")
            this.tcpOut!!.print(message)
            this.tcpOut!!.flush()
        }
    }

    fun stopClient() {
        isConnected = false
        this.tcpSocket!!.close()
    }

    interface OnMessageReceived {
        fun messageReceived(message: String?)
    }

    interface OnConnectEstablished {
        fun onConnected()
    }

    interface OnDisconnected {
        fun onDisconnected()
    }

    init {
        serverPort = port
        mMessageListener = messageListener
        onConnected = onConnect
        onDisconnected = onDisconnect
        try {
            serverIp = InetAddress.getByName(ip)
            serverAddress = InetSocketAddress(serverIp, serverPort)
        } catch (e: UnknownHostException) {
            // TODO Auto-generated catch block
            e.printStackTrace()
        }

    }

}