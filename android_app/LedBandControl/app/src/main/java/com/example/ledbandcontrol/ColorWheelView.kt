package com.example.ledbandcontrol

import android.content.Context
import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.RectF
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.View
import kotlin.math.atan2
import kotlin.math.cos
import kotlin.math.hypot
import kotlin.math.sin

/**
 * A touch-based HSV color wheel. The angle around the center picks the
 * hue, the distance from the center picks the saturation, brightness is
 * fixed at maximum. Tap or drag anywhere inside the circle to pick a color.
 */
class ColorWheelView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null
) : View(context, attrs) {

    companion object {
        // Fixed resolution for the underlying bitmap. It's generated once
        // and scaled to fit the view, so this doesn't need to match the
        // on-screen size and keeps the (somewhat expensive) generation cheap.
        private const val BITMAP_SIZE = 200
        private var cachedWheelBitmap: Bitmap? = null

        private fun getWheelBitmap(): Bitmap {
            return cachedWheelBitmap ?: generateWheelBitmap(BITMAP_SIZE).also {
                cachedWheelBitmap = it
            }
        }

        private fun generateWheelBitmap(size: Int): Bitmap {
            val bitmap = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888)
            val r = size / 2f
            val hsv = floatArrayOf(0f, 0f, 1f)
            for (x in 0 until size) {
                for (y in 0 until size) {
                    val dx = x - r
                    val dy = y - r
                    val distance = hypot(dx.toDouble(), dy.toDouble()).toFloat()
                    if (distance <= r) {
                        var angle = Math.toDegrees(atan2(dy.toDouble(), dx.toDouble())).toFloat()
                        if (angle < 0) angle += 360f
                        hsv[0] = angle
                        hsv[1] = (distance / r).coerceIn(0f, 1f)
                        bitmap.setPixel(x, y, Color.HSVToColor(hsv))
                    } else {
                        bitmap.setPixel(x, y, Color.TRANSPARENT)
                    }
                }
            }
            return bitmap
        }
    }

    var onColorSelected: ((Int) -> Unit)? = null

    private val wheelBitmap = getWheelBitmap()
    private var radius = 0f
    private var centerX = 0f
    private var centerY = 0f

    private var selectorX = 0f
    private var selectorY = 0f
    private var hasSelection = false

    private val bitmapPaint = Paint(Paint.ANTI_ALIAS_FLAG or Paint.FILTER_BITMAP_FLAG)
    private val selectorInnerPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        style = Paint.Style.STROKE
        strokeWidth = 4f
        color = Color.BLACK
    }
    private val selectorOuterPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        style = Paint.Style.STROKE
        strokeWidth = 7f
        color = Color.WHITE
    }

    override fun onSizeChanged(w: Int, h: Int, oldw: Int, oldh: Int) {
        super.onSizeChanged(w, h, oldw, oldh)
        radius = minOf(w, h) / 2f
        centerX = w / 2f
        centerY = h / 2f
        selectorX = centerX
        selectorY = centerY
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        val left = centerX - radius
        val top = centerY - radius
        canvas.drawBitmap(wheelBitmap, null, RectF(left, top, left + radius * 2, top + radius * 2), bitmapPaint)

        if (hasSelection) {
            canvas.drawCircle(selectorX, selectorY, 14f, selectorOuterPaint)
            canvas.drawCircle(selectorX, selectorY, 14f, selectorInnerPaint)
        }
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        if (!isEnabled) return false

        when (event.action) {
            MotionEvent.ACTION_DOWN, MotionEvent.ACTION_MOVE -> {
                var dx = event.x - centerX
                var dy = event.y - centerY
                val distance = hypot(dx.toDouble(), dy.toDouble()).toFloat()

                // Clamp to the edge of the wheel if dragged outside it,
                // so the selector never leaves the circle.
                if (distance > radius && radius > 0f) {
                    val scale = radius / distance
                    dx *= scale
                    dy *= scale
                }

                selectorX = centerX + dx
                selectorY = centerY + dy
                hasSelection = true

                var angle = Math.toDegrees(atan2(dy.toDouble(), dx.toDouble())).toFloat()
                if (angle < 0) angle += 360f
                val saturation = (hypot(dx.toDouble(), dy.toDouble()).toFloat() / radius).coerceIn(0f, 1f)

                val color = Color.HSVToColor(floatArrayOf(angle, saturation, 1f))
                onColorSelected?.invoke(color)
                invalidate()
                return true
            }
        }
        return super.onTouchEvent(event)
    }

    /** Moves the selector indicator to match a color set from elsewhere (e.g. a preset). */
    fun setColor(color: Int) {
        val hsv = FloatArray(3)
        Color.colorToHSV(color, hsv)
        val saturation = hsv[1]
        val angleRad = Math.toRadians(hsv[0].toDouble())
        selectorX = centerX + (saturation * radius * cos(angleRad)).toFloat()
        selectorY = centerY + (saturation * radius * sin(angleRad)).toFloat()
        hasSelection = true
        invalidate()
    }
}