package com.rookiedev.hexapod

import android.app.Activity
import android.app.AlertDialog
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothManager
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.net.InetAddresses.isNumericAddress
import android.os.Bundle
import android.text.Editable
import android.text.TextWatcher
import android.text.method.LinkMovementMethod
import android.view.View
import android.widget.Button
import android.widget.TextView
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.core.app.ActivityCompat
import com.google.android.material.tabs.TabLayout
import com.google.android.material.textfield.TextInputEditText
import com.google.android.material.textfield.TextInputLayout


class MainActivity : AppCompatActivity() {
    companion object {
        private const val BLUETOOTH_PERMISSION_CODE = 100
        private const val BLUETOOTH_SCAN_CODE = 101

        private const val SHARED_PREFS_NAME = "com.rookiedev.hexapod_preferences"
        private const val SHARED_PREFS_IP = "IP"
        private const val SHARED_PREFS_PORT = "PORT"
        private const val SHARED_PREFS_TAB = "TAB"
        private const val SHARED_PREFS_DEVICE_NAME = "DEVICE_NAME"
        private const val SHARED_PREFS_DEVICE_ADDRESS = "DEVICE_ADDRESS"

        private const val TAB_WIFI = "WiFi"
        private const val TAB_BLUETOOTH = "Bluetooth"

        private const val ERROR_NO_PERMISSION = 0
        private const val ERROR_NO_DEVICE = 1
        private const val ERROR_BLUETOOTH_DISABLED = 2
    }

    private var mContext: Context? = null

    private lateinit var tabLayout: TabLayout

    private lateinit var ipInput: TextInputEditText
    private lateinit var portInput: TextInputEditText

    private lateinit var btDeviceName: TextView
    private lateinit var btDeviceMac: TextView

    private var btConnectPermission = false
    private var btScanPermission = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        mContext = applicationContext

        // tab layout
        tabLayout = findViewById(R.id.tab)

        // TCP
        val ipLayout = findViewById<TextInputLayout>(R.id.ip_input_layout)
        val portLayout = findViewById<TextInputLayout>(R.id.port_input_layout)
        ipInput = findViewById(R.id.ip_input)
        portInput = findViewById(R.id.port_input)

        // bluetooth
        val selectedDevice = findViewById<ConstraintLayout>(R.id.selected)
        btDeviceName = findViewById(R.id.textView_device_name)
        btDeviceMac = findViewById(R.id.textView_device_mac)

        // connect button
        val connectButton = findViewById<Button>(R.id.button_connect)

        // GitHub link
        val sourceLink = findViewById<TextView>(R.id.textView_github)
        sourceLink.movementMethod = LinkMovementMethod.getInstance()

        // read shared preference
        readSharedPref()

        // tab select listener
        tabLayout.addOnTabSelectedListener(
            object : TabLayout.OnTabSelectedListener {
                override fun onTabSelected(tab: TabLayout.Tab?) {
                    if (tab!!.text == TAB_WIFI) {
                        ipLayout.visibility = View.VISIBLE
                        portLayout.visibility = View.VISIBLE
                        selectedDevice.visibility = View.GONE
                    } else if (tab.text == TAB_BLUETOOTH) {
                        // request bluetooth permissions
                        checkPermission(
                            "android.permission.BLUETOOTH_CONNECT",
                            BLUETOOTH_PERMISSION_CODE
                        )
                        checkPermission("android.permission.BLUETOOTH_SCAN", BLUETOOTH_SCAN_CODE)
                        ipLayout.visibility = View.GONE
                        portLayout.visibility = View.GONE
                        selectedDevice.visibility = View.VISIBLE
                    }
                }

                override fun onTabUnselected(tab: TabLayout.Tab?) {
                }

                override fun onTabReselected(tab: TabLayout.Tab?) {
                }
            }
        )

        // select bluetooth device
        selectedDevice.setOnClickListener {
            val bluetoothManager =
                this.mContext!!.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
            val bluetoothAdapter: BluetoothAdapter? = bluetoothManager.adapter
            if (bluetoothAdapter != null) {
                if (!bluetoothAdapter.isEnabled) {
                    alertDialog(ERROR_BLUETOOTH_DISABLED)
                } else {
                    if (btConnectPermission && btScanPermission) {
                        val serverIntent = Intent(this, DeviceListActivity::class.java)
                        resultLauncher.launch(serverIntent)
                    } else {
                        alertDialog(ERROR_NO_PERMISSION)
                    }
                }
            }
        }

        if (tabLayout.selectedTabPosition == 0) {
            ipLayout.visibility = View.VISIBLE
            portLayout.visibility = View.VISIBLE
            selectedDevice.visibility = View.GONE
        } else if (tabLayout.selectedTabPosition == 1) {
            // request bluetooth permissions
            checkPermission("android.permission.BLUETOOTH_CONNECT", BLUETOOTH_PERMISSION_CODE)
            checkPermission("android.permission.BLUETOOTH_SCAN", BLUETOOTH_SCAN_CODE)
            ipLayout.visibility = View.GONE
            portLayout.visibility = View.GONE
            selectedDevice.visibility = View.VISIBLE
        }

        connectButton.setOnClickListener {
            // your code to perform when the user clicks on the button

//            Toast.makeText(this@MainActivity, "You clicked me.", Toast.LENGTH_SHORT).show()
            if (tabLayout.selectedTabPosition == 0) {
                if (isNumericAddress(ipInput.text.toString()) && portInput.text.toString()
                        .toInt() >= 0 && portInput.text.toString().toInt() <= 65535
                ) {
                    saveSharedPref()
                    val intent = Intent(this, ControlActivity::class.java).apply {
                        putExtra("interface", TAB_WIFI)
                        putExtra("ip", ipInput.text.toString())
                        putExtra("port", portInput.text.toString())
                    }
                    startActivity(intent)
                } else if (!isNumericAddress(ipInput.text.toString())) {
                    ipLayout.error = getString(R.string.invalid_ip)
                } else {
                    portLayout.error = getString(R.string.invalid_port)
                }
            } else if (tabLayout.selectedTabPosition == 1) {
                if (btDeviceMac.text.isNotBlank()) {
                    val bluetoothManager =
                        this.mContext!!.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
                    val bluetoothAdapter: BluetoothAdapter? = bluetoothManager.adapter
                    if (bluetoothAdapter != null) {
                        if (!bluetoothAdapter.isEnabled) {
                            alertDialog(ERROR_BLUETOOTH_DISABLED)
                        } else {
                            if (btConnectPermission && btScanPermission) {
                                saveSharedPref()
                                val intent = Intent(this, ControlActivity::class.java).apply {
                                    putExtra("interface", TAB_BLUETOOTH)
                                    putExtra("mac", btDeviceMac.text.toString())
                                }
                                startActivity(intent)
                            } else {
                                alertDialog(ERROR_NO_PERMISSION)
                            }
                        }
                    }
                } else {
                    alertDialog(ERROR_NO_DEVICE)
                }
            }
        }


        ipInput.addTextChangedListener(object : TextWatcher {
            override fun onTextChanged(s: CharSequence, start: Int, before: Int, count: Int) {
                ipLayout.error = null
            }

            override fun beforeTextChanged(s: CharSequence, start: Int, count: Int, after: Int) {}
            override fun afterTextChanged(s: Editable) {}
        })

        portInput.addTextChangedListener(object : TextWatcher {
            override fun onTextChanged(s: CharSequence, start: Int, before: Int, count: Int) {
                portLayout.error = null
            }

            override fun beforeTextChanged(s: CharSequence, start: Int, count: Int, after: Int) {}
            override fun afterTextChanged(s: Editable) {}
        })
    }

    private var resultLauncher =
        registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->
            if (result.resultCode == Activity.RESULT_OK) {
                // There are no request codes
                val data: Intent? = result.data
                btDeviceName.text = data!!.getStringExtra("device_name")
                btDeviceMac.text = data.getStringExtra("device_address")
            }
        }

    // Function to check and request permission.
    private fun checkPermission(permission: String, requestCode: Int) {
        println(permission)
        if (ActivityCompat.checkSelfPermission(
                this@MainActivity,
                permission
            ) == PackageManager.PERMISSION_DENIED
        ) {
            // Requesting the permission
            ActivityCompat.requestPermissions(this@MainActivity, arrayOf(permission), requestCode)
        } else {
            // Permission already granted
            if (requestCode == BLUETOOTH_PERMISSION_CODE) {
                btConnectPermission = true
            } else if (requestCode == BLUETOOTH_SCAN_CODE) {
                btScanPermission = true
            }
        }
    }

    // This function is called when the user accepts or decline the permission.
    // Request Code is used to check which permission called this function.
    // This request code is provided when the user is prompt for permission.
    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == BLUETOOTH_PERMISSION_CODE) {
            btConnectPermission =
                grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED
        } else if (requestCode == BLUETOOTH_SCAN_CODE) {
            println("scan permission")
            btScanPermission =
                grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED
        }
    }

    override fun onPause() {
        super.onPause()
        saveSharedPref()
    }


    private fun readSharedPref() {
        val prefs = getSharedPreferences(
            SHARED_PREFS_NAME,
            MODE_PRIVATE
        )
        // read values from the shared preferences
        ipInput.setText(prefs.getString(SHARED_PREFS_IP, "192.168.1.127"))
        portInput.setText(prefs.getString(SHARED_PREFS_PORT, "1234"))

        val selectedTab = prefs.getString(SHARED_PREFS_TAB, TAB_WIFI)
        if (selectedTab == TAB_WIFI) {
            val tab = tabLayout.getTabAt(0)
            tab!!.select()
        } else if (selectedTab == TAB_BLUETOOTH) {
            val tab = tabLayout.getTabAt(1)
            tab!!.select()
        }

        btDeviceName.text = prefs.getString(SHARED_PREFS_DEVICE_NAME, "Click to select a device")
        btDeviceMac.text = prefs.getString(SHARED_PREFS_DEVICE_ADDRESS, "")
    }

    private fun saveSharedPref() {
        val prefs = getSharedPreferences(
            SHARED_PREFS_NAME,
            MODE_PRIVATE
        )
        val editor = prefs.edit()
        editor.putString(SHARED_PREFS_IP, ipInput.text.toString())
        editor.putString(SHARED_PREFS_PORT, portInput.text.toString())
        if (tabLayout.selectedTabPosition == 0) {
            editor.putString(SHARED_PREFS_TAB, TAB_WIFI)
        } else if (tabLayout.selectedTabPosition == 1) {
            editor.putString(SHARED_PREFS_TAB, TAB_BLUETOOTH)
        }

        editor.putString(SHARED_PREFS_DEVICE_NAME, btDeviceName.text.toString())
        editor.putString(SHARED_PREFS_DEVICE_ADDRESS, btDeviceMac.text.toString())

        editor.apply()
    }

    private fun alertDialog(type: Int) {
        val alert: AlertDialog = AlertDialog.Builder(this).create()
        when (type) {
            ERROR_NO_PERMISSION -> {
                alert.setTitle("Permission Error")
                alert.setIcon(R.drawable.ic_baseline_error_24)
                alert.setMessage(
                    "Bluetooth permission is required. Please enable the permission in Settings."
                )
                alert.setOnCancelListener { }
                alert.setButton(
                    AlertDialog.BUTTON_POSITIVE,
                    "OK"
                ) { _, _ -> }
            }
            ERROR_NO_DEVICE -> {
                alert.setTitle("Empty Device")
                alert.setIcon(R.drawable.ic_baseline_error_24)
                alert.setMessage(
                    "Please select a Bluetooth device."
                )
                alert.setOnCancelListener { }
                alert.setButton(
                    AlertDialog.BUTTON_POSITIVE,
                    "OK"
                ) { _, _ -> }
            }
            ERROR_BLUETOOTH_DISABLED -> {
                alert.setTitle("Bluetooth Disabled")
                alert.setIcon(R.drawable.ic_baseline_error_24)
                alert.setMessage(
                    "Please enable Bluetooth in Settings"
                )
                alert.setOnCancelListener { }
                alert.setButton(
                    AlertDialog.BUTTON_POSITIVE,
                    "OK"
                ) { _, _ -> }
            }
        }
        alert.show()
    }
}