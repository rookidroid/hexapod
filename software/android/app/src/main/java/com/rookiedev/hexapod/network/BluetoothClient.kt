package com.rookiedev.hexapod.network

import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothSocket
import android.content.Context
import java.io.*
import java.lang.reflect.InvocationTargetException
import java.lang.reflect.Method
import java.net.*
import java.util.*

class BluetoothClient(
    context: Context?,
    mac: String?,
    messageListener: OnMessageReceived?,
    onConnect: OnConnectEstablished?,
    onDisconnect: OnDisconnected?
) :
    Thread() {
    private var mContext: Context? = null
    private var bluetoothManager: BluetoothManager? = null
    private var bluetoothAdapter: BluetoothAdapter? = null
    private var device: BluetoothDevice? = null

    private var socket: BluetoothSocket? = null

    private var btOut: PrintWriter? = null
    private var btIn: BufferedReader? = null
    private var mMessageListener: OnMessageReceived? = null
    private var onConnected: OnConnectEstablished? = null
    private var onDisconnected: OnDisconnected? = null
    private var isConnected = false

    @SuppressLint("MissingPermission")
    override fun run() {
        val uuid: UUID =
            UUID.fromString("00001101-0000-1000-8000-00805F9B34FB") //UUID for serial connection
        val secure = false
        val port = 10
        try {
            if (secure) {
                if (port == 0) {
                    socket = device!!.createRfcommSocketToServiceRecord(uuid)
                } else {
                    val createRfcommSocket: Method = this.device!!.javaClass.getMethod(
                        "createRfcommSocket", *arrayOf<Class<*>?>(
                            Int::class.javaPrimitiveType
                        )
                    )
                    socket = createRfcommSocket.invoke(device, port) as BluetoothSocket
                }
            } else {
                if (port == 0) {
                    socket = device!!.createInsecureRfcommSocketToServiceRecord(
                        uuid
                    )
                } else {
                    val createInsecureRfcommSocket: Method =
                        this.device!!.javaClass.getMethod(
                            "createInsecureRfcommSocket", *arrayOf<Class<*>?>(
                                Int::class.javaPrimitiveType
                            )
                        )
                    socket = createInsecureRfcommSocket.invoke(
                        device,
                        port
                    ) as BluetoothSocket
                }
            }
        } catch (e: IOException) {
            println(e)
//                    Log.e(TAG, "Socket Type: " + mSocketType.toString() + "create() failed", e)
        } catch (e: NoSuchMethodException) {
            println(e)
//                    Log.e(TAG, "Socket Type: " + mSocketType.toString() + "create() failed", e)
        } catch (e: InvocationTargetException) {
            println(e)
//                    Log.e(TAG, "Socket Type: " + mSocketType.toString() + "create() failed", e)
        } catch (e: IllegalAccessException) {
            println(e)
//                    Log.e(TAG, "Socket Type: " + mSocketType.toString() + "create() failed", e)
        }

        try {
            socket!!.connect()
            onConnected!!.onConnected()
            btOut = PrintWriter(
                BufferedWriter(OutputStreamWriter(socket!!.outputStream)),
                true
            )
            btIn = BufferedReader(InputStreamReader(socket!!.inputStream))
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
        } catch (e: IOException) {
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
        if (this.btOut != null && !this.btOut!!.checkError()) {
            println("send message")
            this.btOut!!.print(message)
            this.btOut!!.flush()
        }
    }

    fun stopClient() {
        isConnected = false
        socket!!.close()
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
        this.mContext = context
        this.mMessageListener = messageListener
        this.onConnected = onConnect
        this.onDisconnected = onDisconnect

        this.bluetoothManager = this.mContext!!.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        this.bluetoothAdapter = this.bluetoothManager!!.adapter

        this.device = this.bluetoothAdapter!!.getRemoteDevice(mac)
    }

}