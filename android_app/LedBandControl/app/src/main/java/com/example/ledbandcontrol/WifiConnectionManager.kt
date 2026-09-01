package com.example.ledbandcontrol

import android.content.Context
import android.net.ConnectivityManager
import android.net.Network
import android.net.NetworkCapabilities
import android.net.NetworkRequest
import android.net.wifi.WifiNetworkSpecifier

/**
 * Connects the app specifically to the D1 mini's access point using
 * WifiNetworkSpecifier, WITHOUT switching the phone's default network.
 * Other apps keep using mobile data / the home WiFi as usual; only this
 * app's own network calls are routed through the LED strip's network
 * once bindProcessToNetwork() is called.
 *
 * The AP has no internet access, which is expected and fine here.
 */
class WifiConnectionManager(context: Context) {

    companion object {
        // Must match utils/secrets.h (AP_SSID / AP_PASSWORD) on the D1 mini
        const val AP_SSID = "LedBandControl"
        const val AP_PASSWORD = "changeme123"
    }

    private val connectivityManager =
        context.applicationContext.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager

    private var networkCallback: ConnectivityManager.NetworkCallback? = null

    fun connect(
        onConnected: () -> Unit,
        onFailed: () -> Unit,
        onLost: () -> Unit
    ) {
        // If a previous callback is still registered, clean it up first.
        disconnect()

        val specifier = WifiNetworkSpecifier.Builder()
            .setSsid(AP_SSID)
            .setWpa2Passphrase(AP_PASSWORD)
            .build()

        val request = NetworkRequest.Builder()
            .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
            .removeCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
            .setNetworkSpecifier(specifier)
            .build()

        val callback = object : ConnectivityManager.NetworkCallback() {
            override fun onAvailable(network: Network) {
                connectivityManager.bindProcessToNetwork(network)
                onConnected()
            }

            override fun onUnavailable() {
                onFailed()
            }

            override fun onLost(network: Network) {
                connectivityManager.bindProcessToNetwork(null)
                onLost()
            }
        }

        networkCallback = callback
        connectivityManager.requestNetwork(request, callback)
    }

    /** Releases the network request and lets the phone go back to its normal network. */
    fun disconnect() {
        networkCallback?.let {
            try {
                connectivityManager.unregisterNetworkCallback(it)
            } catch (e: IllegalArgumentException) {
                // callback was already unregistered, ignore
            }
        }
        networkCallback = null
        connectivityManager.bindProcessToNetwork(null)
    }
}
