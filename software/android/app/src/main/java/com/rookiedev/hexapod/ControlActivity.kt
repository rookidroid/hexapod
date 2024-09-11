package com.rookiedev.hexapod

import android.annotation.SuppressLint
import android.app.AlertDialog
import android.content.Context
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.*
import android.widget.ImageView
import androidx.appcompat.app.AppCompatActivity
import androidx.constraintlayout.widget.ConstraintLayout
import com.rookiedev.hexapod.network.BluetoothClient
import com.rookiedev.hexapod.network.TCPClient
import kotlinx.coroutines.*
import kotlin.math.PI
import kotlin.math.atan2
import kotlin.math.pow
import kotlin.math.sqrt


/**
 * Behaviors of immersive mode.
 */
enum class BehaviorOption(
    val title: String,
    val value: Int
) {
    // Swipe from the edge to show a hidden bar. Gesture navigation works regardless of visibility
    // of the navigation bar.
    Default(
        "BEHAVIOR_DEFAULT",
        WindowInsetsController.BEHAVIOR_DEFAULT
    ),

    // "Sticky immersive mode". Swipe from the edge to temporarily reveal the hidden bar.
    ShowTransientBarsBySwipe(
        "BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE",
        WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
    )
}

/**
 * Type of system bars to hide or show.
 */
enum class TypeOption(
    val title: String,
    val value: Int
) {
    // Both the status bar and the navigation bar
    SystemBars(
        "systemBars()",
        WindowInsets.Type.systemBars()
    ),

    // The status bar only.
    StatusBar(
        "statusBars()",
        WindowInsets.Type.statusBars()
    ),

    // The navigation bar only
    NavigationBar(
        "navigationBars()",
        WindowInsets.Type.navigationBars()
    )
}

class ControlActivity : AppCompatActivity() {
    companion object {
        private const val CMD_STANDBY = "standby:"
        private const val CMD_LAYDOWN = "laydown:"

        private const val CMD_WALK_0 = "walk0:"
        private const val CMD_WALK_180 = "walk180:"

        private const val CMD_WALK_R45 = "walkr45:"
        private const val CMD_WALK_R90 = "walkr90:"
        private const val CMD_WALK_R135 = "walkr135:"

        private const val CMD_WALK_L45 = "walkl45:"
        private const val CMD_WALK_L90 = "walkl90:"
        private const val CMD_WALK_L135 = "walkl135:"

        private const val CMD_FASTFORWARD = "fastforward:"
        private const val CMD_FASTBACKWARD = "fastbackward:"

        private const val CMD_TURNLEFT = "turnleft:"
        private const val CMD_TURNRIGHT = "turnright:"

        private const val CMD_CLIMBFORWARD = "climbforward:"
        private const val CMD_CLIMBBACKWARD = "climbbackward:"

        private const val CMD_ROTATEX = "rotatex:"
        private const val CMD_ROTATEY = "rotatey:"
        private const val CMD_ROTATEZ = "rotatez:"

        private const val CMD_TWIST = "twist:"

        private const val CTRL_LEFT = 1
        private const val CTRL_RIGHT = 0
    }

    private var rightWidth = 0
    private var rightHeight = 0
    private var rightRadius = 0f

    private var leftWidth = 0
    private var leftHeight = 0

    private var connectInterface: String = ""

    private var mContext: Context? = null

    private var tcpClient: TCPClient? = null
    private var ip: String = ""
    private var port = 0

    private var btClient: BluetoothClient? = null
    private var mac: String = ""

    private val scope = CoroutineScope(Job() + Dispatchers.IO)

    private var currentState: String = CMD_STANDBY
    private lateinit var progressBar: ConstraintLayout

    private var rightControlImage: ImageView? = null
    private var leftControlImage: ImageView? = null

    private var controlImg = CTRL_RIGHT

    @SuppressLint("ClickableViewAccessibility")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_control)

        val myIntent = intent // gets the previously created intent

        mContext = applicationContext

        connectInterface = myIntent.getStringExtra("interface").toString()
        if (connectInterface == "WiFi") {
            ip = myIntent.getStringExtra("ip").toString()
            port = myIntent.getStringExtra("port").toString().toInt()
        } else if (connectInterface == "Bluetooth") {
            mac = myIntent.getStringExtra("mac").toString()
        }

        controlWindowInsets(true)

        rightControlImage = findViewById(R.id.right_control_image)
        leftControlImage = findViewById(R.id.left_control_image)
        progressBar = findViewById(R.id.progressBar)

        val rightVto: ViewTreeObserver = rightControlImage!!.viewTreeObserver
        rightVto.addOnPreDrawListener(object : ViewTreeObserver.OnPreDrawListener {
            override fun onPreDraw(): Boolean {
                rightControlImage!!.viewTreeObserver.removeOnPreDrawListener(this)
                rightHeight = rightControlImage!!.measuredHeight
                rightWidth = rightControlImage!!.measuredWidth
                rightRadius = rightWidth.coerceAtMost(rightHeight) / 2f
                return true
            }
        })

        val leftVto: ViewTreeObserver = leftControlImage!!.viewTreeObserver
        leftVto.addOnPreDrawListener(object : ViewTreeObserver.OnPreDrawListener {
            override fun onPreDraw(): Boolean {
                leftControlImage!!.viewTreeObserver.removeOnPreDrawListener(this)
                leftHeight = leftControlImage!!.measuredHeight
                leftWidth = 2 * leftHeight / 3
                return true
            }
        })

        rightControlImage!!.setOnTouchListener(
            object : View.OnTouchListener {
                override fun onTouch(view: View, motionEvent: MotionEvent): Boolean {
                    when (motionEvent.action) {
                        MotionEvent.ACTION_MOVE, MotionEvent.ACTION_DOWN -> {
                            if (motionEvent.action == MotionEvent.ACTION_DOWN) {
                                controlImg = CTRL_RIGHT
                            }
                            if (controlImg == CTRL_RIGHT) {
                                val touchX = motionEvent.x
                                val touchY = motionEvent.y
                                if (touchX < 0) {
                                    return false
                                }
                                if (touchY < 0) {
                                    return false
                                }

                                val coorX = touchX - rightWidth / 2
                                val coorY = touchY - rightHeight / 2

                                val length = sqrt(coorX.pow(2) + coorY.pow(2))
                                if (length < rightRadius / 4) {
                                    if (currentState != CMD_STANDBY) {
                                        sendMessageAsync(CMD_STANDBY)
                                        currentState = CMD_STANDBY
                                        rightControlImage!!.setImageResource(R.drawable.ic_control_circle_standby)

                                        leftControlImage!!.setImageResource(R.drawable.ic_control_left)
                                    }
                                } else if (length >= rightRadius / 4 && length < 2 * rightRadius / 3) {
                                    val angle = atan2(coorY, coorX)
                                    if (angle > -7 * PI / 8 && angle < -5 * PI / 8) {
                                        if (currentState != CMD_WALK_L45) {
                                            sendMessageAsync(CMD_WALK_L45)
                                            currentState = CMD_WALK_L45
                                            rightControlImage!!.setImageResource(R.drawable.ic_control_circle_walk_l45)
                                        }
                                    } else if (angle > -5 * PI / 8 && angle < -3 * PI / 8) {
                                        if (currentState != CMD_WALK_0) {
                                            sendMessageAsync(CMD_WALK_0)
                                            currentState = CMD_WALK_0
                                            rightControlImage!!.setImageResource(R.drawable.ic_control_circle_walk_0)
                                        }
                                    } else if (angle > -3 * PI / 8 && angle <= -PI / 8) {
                                        if (currentState != CMD_WALK_R45) {
                                            sendMessageAsync(CMD_WALK_R45)
                                            currentState = CMD_WALK_R45
                                            rightControlImage!!.setImageResource(R.drawable.ic_control_circle_walk_r45)
                                        }
                                    } else if (angle > -PI / 8 && angle <= PI / 8) {
                                        if (currentState != CMD_WALK_R90) {
                                            sendMessageAsync(CMD_WALK_R90)
                                            currentState = CMD_WALK_R90
                                            rightControlImage!!.setImageResource(R.drawable.ic_control_circle_walk_r90)
                                        }
                                    } else if (angle > PI / 8 && angle <= 3 * PI / 8) {
                                        if (currentState != CMD_WALK_R135) {
                                            sendMessageAsync(CMD_WALK_R135)
                                            currentState = CMD_WALK_R135
                                            rightControlImage!!.setImageResource(R.drawable.ic_control_circle_walk_r135)
                                        }
                                    } else if (angle > 3 * PI / 8 && angle <= 5 * PI / 8) {
                                        if (currentState != CMD_WALK_180) {
                                            sendMessageAsync(CMD_WALK_180)
                                            currentState = CMD_WALK_180
                                            rightControlImage!!.setImageResource(R.drawable.ic_control_circle_walk_180)
                                        }
                                    } else if (angle > 5 * PI / 8 && angle <= 7 * PI / 8) {
                                        if (currentState != CMD_WALK_L135) {
                                            sendMessageAsync(CMD_WALK_L135)
                                            currentState = CMD_WALK_L135
                                            rightControlImage!!.setImageResource(R.drawable.ic_control_circle_walk_l135)
                                        }
                                    } else {
                                        if (currentState != CMD_WALK_L90) {
                                            sendMessageAsync(CMD_WALK_L90)
                                            currentState = CMD_WALK_L90
                                            rightControlImage!!.setImageResource(R.drawable.ic_control_circle_walk_l90)
                                        }
                                    }
                                    leftControlImage!!.setImageResource(R.drawable.ic_control_left)
                                } else if (length >= 2 * rightRadius / 3 && length < rightRadius) {
                                    val angle = atan2(coorY, coorX)
                                    if (angle > -PI / 4 && angle <= PI / 4) {
                                        if (currentState != CMD_TURNRIGHT) {
                                            sendMessageAsync(CMD_TURNRIGHT)
                                            currentState = CMD_TURNRIGHT
                                            rightControlImage!!.setImageResource(R.drawable.ic_control_circle_turnright)
                                        }
                                    } else if (angle > PI / 4 && angle <= 3 * PI / 4) {
                                        if (currentState != CMD_FASTBACKWARD) {
                                            sendMessageAsync(CMD_FASTBACKWARD)
                                            currentState = CMD_FASTBACKWARD
                                            rightControlImage!!.setImageResource(R.drawable.ic_control_circle_fastbackward)
                                        }
                                    } else if (angle > -3 * PI / 4 && angle < -PI / 4) {
                                        if (currentState != CMD_FASTFORWARD) {
                                            sendMessageAsync(CMD_FASTFORWARD)
                                            currentState = CMD_FASTFORWARD
                                            rightControlImage!!.setImageResource(R.drawable.ic_control_circle_fastforward)
                                        }
                                    } else {
                                        if (currentState != CMD_TURNLEFT) {
                                            sendMessageAsync(CMD_TURNLEFT)
                                            currentState = CMD_TURNLEFT
                                            rightControlImage!!.setImageResource(R.drawable.ic_control_circle_turnleft)
                                        }
                                    }
                                    leftControlImage!!.setImageResource(R.drawable.ic_control_left)
                                }
                            }
                        }
                        MotionEvent.ACTION_UP -> {
                            if (controlImg == CTRL_RIGHT) {
                                sendMessageAsync(CMD_STANDBY)
                                currentState = CMD_STANDBY
                                rightControlImage!!.setImageResource(R.drawable.ic_control_circle_standby)

                                leftControlImage!!.setImageResource(R.drawable.ic_control_left)

                                controlImg = CTRL_LEFT
                            }
                        }
                    }
                    return true
                }
            }
        )

        leftControlImage!!.setOnTouchListener(
            object : View.OnTouchListener {
                override fun onTouch(view: View, motionEvent: MotionEvent): Boolean {
                    when (motionEvent.action) {
                        MotionEvent.ACTION_MOVE, MotionEvent.ACTION_DOWN -> {
                            if (motionEvent.action == MotionEvent.ACTION_DOWN) {
                                controlImg = CTRL_LEFT
                            }
                            if (controlImg == CTRL_LEFT) {
                                val touchX = motionEvent.x
                                val touchY = motionEvent.y
                                if (touchX < 0 || touchX > leftWidth) {
                                    return false
                                }
                                if (touchY < 0 || touchY > leftHeight) {
                                    return false
                                }

                                if (touchX < leftWidth / 2) {
                                    if (touchY < leftHeight / 3) {
                                        if (currentState != CMD_ROTATEY) {
                                            sendMessageAsync(CMD_ROTATEY)
                                            currentState = CMD_ROTATEY
                                            leftControlImage!!.setImageResource(R.drawable.ic_control_left_rotatey)
                                        }
                                    } else if (touchY >= leftHeight / 3 && touchY < 2 * leftHeight / 3) {
                                        if (currentState != CMD_ROTATEX) {
                                            sendMessageAsync(CMD_ROTATEX)
                                            currentState = CMD_ROTATEX
                                            leftControlImage!!.setImageResource(R.drawable.ic_control_left_rotatex)
                                        }
                                    } else {
                                        if (currentState != CMD_ROTATEZ) {
                                            sendMessageAsync(CMD_ROTATEZ)
                                            currentState = CMD_ROTATEZ
                                            leftControlImage!!.setImageResource(R.drawable.ic_control_left_rotatez)
                                        }
                                    }
                                } else {
                                    if (touchY < leftHeight / 3) {
                                        if (currentState != CMD_CLIMBFORWARD) {
                                            sendMessageAsync(CMD_CLIMBFORWARD)
                                            currentState = CMD_CLIMBFORWARD
                                            leftControlImage!!.setImageResource(R.drawable.ic_control_left_climb_forward)
                                        }
                                    } else if (touchY >= leftHeight / 3 && touchY < 2 * leftHeight / 3) {
                                        if (currentState != CMD_TWIST) {
                                            sendMessageAsync(CMD_TWIST)
                                            currentState = CMD_TWIST
                                            leftControlImage!!.setImageResource(R.drawable.ic_control_left_twist)
                                        }
                                    } else {
                                        if (currentState != CMD_CLIMBBACKWARD) {
                                            sendMessageAsync(CMD_CLIMBBACKWARD)
                                            currentState = CMD_CLIMBBACKWARD
                                            leftControlImage!!.setImageResource(R.drawable.ic_control_left_climb_backward)
                                        }
                                    }
                                }
                                rightControlImage!!.setImageResource(R.drawable.ic_control_circle)
                            }
                        }
                        MotionEvent.ACTION_UP -> {
                            if (controlImg == CTRL_LEFT) {
                                sendMessageAsync(CMD_STANDBY)
                                currentState = CMD_STANDBY
                                rightControlImage!!.setImageResource(R.drawable.ic_control_circle_standby)

                                leftControlImage!!.setImageResource(R.drawable.ic_control_left)

                                controlImg = CTRL_RIGHT
                            }
                        }
                    }
                    return true
                }
            }
        )
    }

    @SuppressLint("MissingPermission")
    override fun onResume() {
        super.onResume()
        progressBar.visibility = View.VISIBLE

        if (connectInterface == "WiFi") {
            this.tcpClient = TCPClient(ip, port, object : TCPClient.OnMessageReceived {
                override fun messageReceived(message: String?) {
                    if (message == null) {
//                    alertDialog(DISCONNECTED)
                        println("no message")
                    }
                }
            }, object : TCPClient.OnConnectEstablished {
                override fun onConnected() {
//                udpClient.start()
                    println("connected")
                    Handler(Looper.getMainLooper()).post {
                        progressBar.visibility = View.GONE
                    }
                }
            }, object : TCPClient.OnDisconnected {
                override fun onDisconnected() {
                    Handler(Looper.getMainLooper()).post {
                        progressBar.visibility = View.GONE
                        alertDialog(0)
                    }
                }
            }
            )
            this.tcpClient!!.start()
        } else if (connectInterface == "Bluetooth") {
            println("Bluetooth")
            this.btClient =
                BluetoothClient(mContext, mac, object : BluetoothClient.OnMessageReceived {
                    override fun messageReceived(message: String?) {
                        if (message == null) {
//                    alertDialog(DISCONNECTED)
                            println("no message")
                        }
                    }
                }, object : BluetoothClient.OnConnectEstablished {
                    override fun onConnected() {
//                udpClient.start()
                        println("connected")
                        Handler(Looper.getMainLooper()).post {
                            progressBar.visibility = View.GONE
                        }
                    }
                }, object : BluetoothClient.OnDisconnected {
                    override fun onDisconnected() {
                        Handler(Looper.getMainLooper()).post {
                            progressBar.visibility = View.GONE
                            alertDialog(0)
                        }
                    }
                }
                )
            this.btClient!!.start()

        }

        currentState = CMD_STANDBY
        rightControlImage!!.setImageResource(R.drawable.ic_control_circle_standby)
        leftControlImage!!.setImageResource(R.drawable.ic_control_left)
    }

    override fun onPause() {
        super.onPause()
        if (connectInterface == "WiFi") {
            tcpClient!!.stopClient()
            tcpClient!!.interrupt()
        } else if (connectInterface == "Bluetooth") {
            btClient!!.stopClient()
            btClient!!.interrupt()
        }

    }


    private fun controlWindowInsets(hide: Boolean) {
        // WindowInsetsController can hide or show specified system bars.
        val insetsController = window.decorView.windowInsetsController ?: return
        // The behavior of the immersive mode.
        val behavior = BehaviorOption.values()[1].value
        // The type of system bars to hide or show.
        val type = TypeOption.values()[0].value
        insetsController.systemBarsBehavior = behavior
        if (hide) {
            insetsController.hide(type)
        } else {
            insetsController.show(type)
        }
    }

    fun sendMessageAsync(message: String) {
        // Starts a new coroutine within the scope
        scope.launch {
            // New coroutine that can call suspend functions

            withContext(Dispatchers.IO) {              // Dispatchers.IO (main-safety block)
                if (connectInterface == "WiFi") {
                    tcpClient?.sendMessage(message)
                } else if (connectInterface == "Bluetooth") {
                    btClient?.sendMessage(message)
                }
            }
        }
    }

    fun alertDialog(type: Int) {
        val alert: AlertDialog = AlertDialog.Builder(this).create()
        when (type) {
            0 -> {
                alert.setTitle("Error")
                alert.setIcon(R.drawable.ic_baseline_error_24)
                alert.setMessage(
                    "Unable to connect to the Hexapod."
                )
                alert.setOnCancelListener { finish() }
                alert.setButton(
                    AlertDialog.BUTTON_POSITIVE,
                    "OK"
                ) { _, _ -> finish() }
            }
        }
        alert.show()
    }
}


