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
        val width: Int = size.x
        val height: Int = size.y

        start_capturing.setOnClickListener {
            val intent = Intent(this@MainActivity, CameraPreviewActivity::class.java)
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
                        images_layout.height*bitmap.width/bitmap.height,
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
