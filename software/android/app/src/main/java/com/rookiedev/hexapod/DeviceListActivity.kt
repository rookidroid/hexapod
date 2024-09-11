package com.rookiedev.hexapod

import android.annotation.SuppressLint
import android.app.Activity
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.content.Context
import android.content.Intent
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AdapterView.OnItemClickListener
import android.widget.ArrayAdapter
import android.widget.ImageView
import android.widget.ListView
import android.widget.TextView


class DeviceAdapter(mContext: Context?, private val devices: ArrayList<BluetoothDevice>) :
    ArrayAdapter<BluetoothDevice?>(mContext!!, 0, devices as List<BluetoothDevice?>) {
    @SuppressLint("MissingPermission")
    override fun getView(position: Int, convertView: View?, parent: ViewGroup): View {
        // Get the data item for this position
        var cView = convertView
        val device: BluetoothDevice = getItem(position)
        // Check if an existing view is being reused, otherwise inflate the view
        if (cView == null) {
            cView = LayoutInflater.from(context).inflate(R.layout.device_list, parent, false)
        }

        val textDeviceName = cView!!.findViewById<TextView>(R.id.list_device_name)
        val textDeviceAddress = cView.findViewById<TextView>(R.id.list_device_address)
        val bluetoothIcon = cView.findViewById<ImageView>(R.id.bluetooth_icon)

        bluetoothIcon.setImageResource(R.drawable.ic_baseline_bluetooth_24)

        // Populate the data into the template view using the data object
        textDeviceName.text = device.name
        textDeviceAddress.text = device.address
        // Return the completed view to render on screen
        return cView
    }

    override fun getCount(): Int {
        return devices.size
    }

    override fun getItem(arg0: Int): BluetoothDevice {
        return devices[arg0]
    }

    override fun getItemId(arg0: Int): Long {
        return arg0.toLong()
    }
}


class DeviceListActivity : Activity() {

    private var mContext: Context? = null
    private var bluetoothManager: BluetoothManager? = null
    private var bluetoothAdapter: BluetoothAdapter? = null

    @SuppressLint("MissingPermission")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        // Setup the window
        setContentView(R.layout.activity_device_list)

        mContext = applicationContext

        // Set result CANCELED in case the user backs out
        setResult(RESULT_CANCELED)

        // Get the local Bluetooth adapter
        this.bluetoothManager = this.mContext!!.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        this.bluetoothAdapter = this.bluetoothManager!!.adapter

        // Get a set of currently paired devices
        val pairedDevices: Set<BluetoothDevice> = bluetoothAdapter!!.bondedDevices
        val deviceList: ArrayList<BluetoothDevice> = ArrayList(pairedDevices)
        val deviceAdapter = DeviceAdapter(this, deviceList)

        // Find and set up the ListView for paired devices
        val pairedListView: ListView = findViewById(R.id.paired_devices)
        pairedListView.adapter = deviceAdapter
        pairedListView.onItemClickListener = mDeviceClickListener

        val cancelPicker = findViewById<TextView>(R.id.cancel_picker)
        cancelPicker.setOnClickListener {
            finish()
        }
    }

    /**
     * The on-click listener for all devices in the ListViews
     */
    @SuppressLint("MissingPermission")
    private val mDeviceClickListener =
        OnItemClickListener { av, _, arg2, _ ->

            val device: BluetoothDevice = av.getItemAtPosition(arg2) as BluetoothDevice

            // Create the result Intent and include the MAC address
            val intent = Intent()
            intent.putExtra(EXTRA_DEVICE_ADDRESS, device.address)
            intent.putExtra(EXTRA_DEVICE_NAME, device.name)

            // Set result and finish this Activity
            setResult(RESULT_OK, intent)

            finish()
        }


    companion object {
        /**
         * Return Intent extra
         */
        var EXTRA_DEVICE_ADDRESS = "device_address"
        var EXTRA_DEVICE_NAME = "device_name"
    }
}