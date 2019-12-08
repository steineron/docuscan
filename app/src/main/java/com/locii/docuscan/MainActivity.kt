package com.locii.docuscan

import android.content.Intent
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Point
import android.os.Bundle
import android.view.Display
import android.widget.ImageView
import androidx.appcompat.app.AppCompatActivity
import com.locii.docuscanlib.CameraPreviewActivity
import kotlinx.android.synthetic.main.activity_main.*


class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        val display: Display = windowManager.defaultDisplay
        val size = Point()
        display.getSize(size)
        // invert those to create a landscape effect
        val width: Int = size.x
        val height: Int = size.y

        // in the example - the DL is 85x55 mm so assume the width should capture 70% of the screen and the height should match:

        val guidWidth = width * .70
        val guideHeight = guidWidth * 85/55
        val guideTL = Point((width * .15).toInt(), ((height - guideHeight) / 2).toInt())
        val guideBR = Point((guideTL.x + guidWidth).toInt(), (guideTL.y + guideHeight).toInt())



        start_capturing.setOnClickListener {
            val tlbr = ArrayList<Int>()
            with(tlbr) {
                add(guideTL.x)
                add(guideTL.y)
                add(guideBR.x)
                add(guideBR.y)
            }
            val intent = Intent(
                this@MainActivity,
                CameraPreviewActivity::class.java
            ).putIntegerArrayListExtra(
                "tlbr",
                tlbr
            )

            startActivityForResult(intent, 126)
        }

    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)

        data?.getStringArrayExtra("paths")?.let { paths ->

            paths.forEachIndexed { index, path ->

                var bitmap = BitmapFactory.decodeFile(path)
                images_layout.getChildAt(index)?.let { imageview ->
                    imageview.post {
                        bitmap = Bitmap.createScaledBitmap(
                            bitmap,
                            images_layout.height * bitmap.width / bitmap.height,
                            images_layout.height,
                            true
                        )
                        (imageview as ImageView).setImageBitmap(bitmap)
                    }
                }
            }
        }
    }

}
