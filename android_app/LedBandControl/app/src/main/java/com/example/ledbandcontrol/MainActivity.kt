package com.example.ledbandcontrol

import android.Manifest
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.graphics.Color
import android.graphics.drawable.GradientDrawable
import android.net.wifi.WifiManager
import android.os.Bundle
import android.provider.Settings
import android.widget.Button
import android.widget.GridLayout
import android.widget.TextView
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat

class MainActivity : AppCompatActivity() {

    // 16 static colors matching a typical cheap LED strip remote (4x4 grid).
    private val presetColors = listOf(
        Triple(255, 0, 0), Triple(255, 51, 0), Triple(51, 204, 51), Triple(26, 26, 179),
        Triple(255, 153, 0), Triple(51, 204, 204), Triple(102, 0, 204), Triple(153, 51, 255),
        Triple(255, 204, 102), Triple(51, 153, 255), Triple(153, 0, 204), Triple(255, 51, 153),
        Triple(255, 255, 51), Triple(0, 128, 128), Triple(255, 20, 147), Triple(255, 182, 193)
    )

    private val modeButtonLabels = mapOf(
        "flash" to "FLASH",
        "strobe" to "STROBE",
        "fade" to "FADE",
        "smooth" to "SMOOTH"
    )

    private lateinit var wifiManager: WifiConnectionManager
    private lateinit var apiClient: LedApiClient
    private lateinit var systemWifiManager: WifiManager

    private lateinit var statusText: TextView
    private lateinit var connectButton: Button
    private lateinit var brightnessMinusButton: Button
    private lateinit var brightnessPlusButton: Button
    private lateinit var onButton: Button
    private lateinit var offButton: Button
    private lateinit var rButton: Button
    private lateinit var gButton: Button
    private lateinit var bButton: Button
    private lateinit var wButton: Button
    private lateinit var presetGrid: GridLayout
    private lateinit var modeButtons: Map<String, Button>

    private var isConnected = false
    private var activeMode: String? = null // null = static

    // Remembers the last non-off color so the ON button can restore it.
    private var lastColor = Triple(255, 255, 255)

    private val requestLocationPermission = registerForActivityResult(
        ActivityResultContracts.RequestPermission()
    ) { granted ->
        if (granted) startConnection()
        else setStatus("Location permission is required to connect")
    }

    private val wifiSettingsLauncher = registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) {
        if (systemWifiManager.isWifiEnabled) requestConnection()
        else setStatus("WiFi is still off, turn it on to connect")
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        wifiManager = WifiConnectionManager(this)
        apiClient = LedApiClient()
        systemWifiManager = applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager

        statusText = findViewById(R.id.statusText)
        connectButton = findViewById(R.id.connectButton)
        brightnessMinusButton = findViewById(R.id.brightnessMinusButton)
        brightnessPlusButton = findViewById(R.id.brightnessPlusButton)
        onButton = findViewById(R.id.onButton)
        offButton = findViewById(R.id.offButton)
        rButton = findViewById(R.id.rButton)
        gButton = findViewById(R.id.gButton)
        bButton = findViewById(R.id.bButton)
        wButton = findViewById(R.id.wButton)
        presetGrid = findViewById(R.id.presetGrid)

        modeButtons = mapOf(
            "flash" to findViewById(R.id.modeFlashButton),
            "strobe" to findViewById(R.id.modeStrobeButton),
            "fade" to findViewById(R.id.modeFadeButton),
            "smooth" to findViewById(R.id.modeSmoothButton)
        )

        connectButton.setOnClickListener {
            if (isConnected) {
                disconnectFromStrip()
            } else if (systemWifiManager.isWifiEnabled) {
                requestConnection()
            } else {
                setStatus("Turn on WiFi to continue")
                wifiSettingsLauncher.launch(Intent(Settings.Panel.ACTION_WIFI))
            }
        }

        brightnessMinusButton.setOnClickListener { apiClient.adjustBrightness(-10) { handleSimpleResult(it) } }
        brightnessPlusButton.setOnClickListener { apiClient.adjustBrightness(10) { handleSimpleResult(it) } }

        onButton.setOnClickListener { val (r, g, b) = lastColor; applyColor(r, g, b) }
        offButton.setOnClickListener { applyColor(0, 0, 0, rememberAsLastColor = false) }

        rButton.setOnClickListener { applyColor(255, 0, 0) }
        gButton.setOnClickListener { applyColor(0, 255, 0) }
        bButton.setOnClickListener { applyColor(0, 0, 255) }
        wButton.setOnClickListener { applyColor(255, 255, 255) }

        for ((mode, button) in modeButtons) {
            button.setOnClickListener { toggleMode(mode) }
        }

        buildPresetButtons()
        setControlsEnabled(false)
        setStatus("Disconnected")
    }

    private fun buildPresetButtons() {
        val sizePx = dpToPx(40)
        val marginPx = dpToPx(4)

        for ((r, g, b) in presetColors) {
            val swatch = android.view.View(this)
            val params = GridLayout.LayoutParams().apply {
                width = sizePx
                height = sizePx
                setMargins(marginPx, marginPx, marginPx, marginPx)
            }
            swatch.layoutParams = params
            swatch.background = GradientDrawable().apply {
                shape = GradientDrawable.OVAL
                setColor(Color.rgb(r, g, b))
            }
            swatch.setOnClickListener { applyColor(r, g, b) }
            presetGrid.addView(swatch)
        }
    }

    private fun requestConnection() {
        val hasPermission = ContextCompat.checkSelfPermission(
            this, Manifest.permission.ACCESS_FINE_LOCATION
        ) == PackageManager.PERMISSION_GRANTED

        if (hasPermission) startConnection()
        else requestLocationPermission.launch(Manifest.permission.ACCESS_FINE_LOCATION)
    }

    private fun startConnection() {
        setStatus("Connecting to ${WifiConnectionManager.AP_SSID}...")
        connectButton.isEnabled = false

        wifiManager.connect(
            onConnected = {
                runOnUiThread {
                    isConnected = true
                    connectButton.isEnabled = true
                    connectButton.text = "Disconnect"
                    setControlsEnabled(true)
                    setStatus("Connected")
                }
            },
            onFailed = {
                runOnUiThread {
                    connectButton.isEnabled = true
                    setStatus("Couldn't find or join ${WifiConnectionManager.AP_SSID}")
                }
            },
            onLost = {
                runOnUiThread {
                    isConnected = false
                    connectButton.text = "Connect"
                    setControlsEnabled(false)
                    setStatus("Connection lost")
                }
            }
        )
    }

    private fun disconnectFromStrip() {
        wifiManager.disconnect()
        isConnected = false
        connectButton.text = "Connect"
        setControlsEnabled(false)
        setStatus("Disconnected")
    }

    private fun toggleMode(mode: String) {
        val newMode = if (activeMode == mode) "static" else mode
        apiClient.setMode(newMode) { success ->
            runOnUiThread {
                if (success) {
                    activeMode = if (newMode == "static") null else newMode
                    updateModeButtonLabels()
                } else {
                    setStatus("Request failed, still connected?")
                }
            }
        }
    }

    private fun updateModeButtonLabels() {
        for ((mode, button) in modeButtons) {
            val baseLabel = modeButtonLabels.getValue(mode)
            button.text = if (activeMode == mode) "$baseLabel ✓" else baseLabel
        }
    }

    // Used by presets, R/G/B/W and ON/OFF: sends the color right away and
    // cancels any running effect mode, same as a real remote's color buttons.
    private fun applyColor(r: Int, g: Int, b: Int, rememberAsLastColor: Boolean = true) {
        if (activeMode != null) {
            activeMode = null
            updateModeButtonLabels()
        }
        if (rememberAsLastColor) {
            lastColor = Triple(r, g, b)
        }

        apiClient.setColor(r, g, b) { success ->
            runOnUiThread {
                if (!success) setStatus("Request failed, still connected?")
            }
        }
    }

    private fun handleSimpleResult(success: Boolean) {
        runOnUiThread {
            if (!success) setStatus("Request failed, still connected?")
        }
    }

    private fun setControlsEnabled(enabled: Boolean) {
        brightnessMinusButton.isEnabled = enabled
        brightnessPlusButton.isEnabled = enabled
        onButton.isEnabled = enabled
        offButton.isEnabled = enabled
        rButton.isEnabled = enabled
        gButton.isEnabled = enabled
        bButton.isEnabled = enabled
        wButton.isEnabled = enabled
        for (button in modeButtons.values) button.isEnabled = enabled
        for (i in 0 until presetGrid.childCount) presetGrid.getChildAt(i).isEnabled = enabled
    }

    private fun setStatus(text: String) {
        statusText.text = text
    }

    private fun dpToPx(dp: Int): Int = (dp * resources.displayMetrics.density).toInt()

    override fun onDestroy() {
        super.onDestroy()
        wifiManager.disconnect()
    }
}