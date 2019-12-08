/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.locii.docuscanlib

import android.content.Context
import android.graphics.*
import android.view.View

/**
 * GuideView
 *
 * @author steineron (Ron Steiner)
 * @version 1
 */


class GuideView(context: Context) : View(context) {

    val paint = Paint()

    init {
        paint.style = Paint.Style.STROKE
        paint.color = Color.GREEN
        paint.strokeWidth = 2.5f
    }

    private var rect: RectF? = null

    var topLeft: PointF? = null
        set(value) {
            field = value
            rect = bottomRight?.let {
                if (value != null) {
                    RectF(value.x, value.y, it.x, it.y)
                } else null
            }
        }


    var bottomRight: PointF? = null
        set(value) {
            field = value
            rect = topLeft?.let {
                if (value != null) {
                    RectF(it.x, it.y, value.x, value.y)
                } else null
            }
        }

    override fun onDraw(canvas: Canvas?) {
        super.onDraw(canvas)

        rect?.let {

            canvas?.drawRoundRect(it, 12.0f, 12.0f, paint)
        }

    }
}