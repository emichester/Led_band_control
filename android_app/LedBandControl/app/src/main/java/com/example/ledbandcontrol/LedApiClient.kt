package com.example.ledbandcontrol

import android.util.Log
import java.net.HttpURLConnection
import java.net.URL
import java.util.concurrent.Executors

/**
 * Talks to the D1 mini's web server (see utils/web_server.h in the
 * firmware project). All requests run on a background thread, since
 * network I/O is not allowed on Android's main thread.
 */
class LedApiClient(private val baseUrl: String = "http://192.168.4.1") {

    private val executor = Executors.newSingleThreadExecutor()

    fun setColor(r: Int, g: Int, b: Int, onResult: (Boolean) -> Unit) {
        get("/setColor?r=$r&g=$g&b=$b", onResult)
    }

    fun setMode(mode: String, onResult: (Boolean) -> Unit) {
        get("/setMode?mode=$mode", onResult)
    }

    fun adjustBrightness(delta: Int, onResult: (Boolean) -> Unit) {
        get("/adjustBrightness?delta=$delta", onResult)
    }

    private fun get(path: String, onResult: (Boolean) -> Unit) {
        executor.execute {
            val success = try {
                val connection = URL(baseUrl + path).openConnection() as HttpURLConnection
                connection.connectTimeout = 3000
                connection.readTimeout = 3000
                connection.requestMethod = "GET"
                val ok = connection.responseCode == 200
                connection.disconnect()
                ok
            } catch (e: Exception) {
                // Check Logcat (filter: LedApiClient) if requests keep failing.
                Log.e("LedApiClient", "Request to $baseUrl$path failed", e)
                false
            }
            onResult(success)
        }
    }
}