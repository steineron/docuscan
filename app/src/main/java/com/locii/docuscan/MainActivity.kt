package com.locii.docuscan

import android.content.Intent
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.locii.docuscanlib.CameraPreviewActivity
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        start_capturing.setOnClickListener {
            val intent = Intent(this@MainActivity, CameraPreviewActivity::class.java)
            startActivityForResult(intent, 126)

        }

    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)

        data?.getStringExtra("path")?.let {

            var bitmap = BitmapFactory.decodeFile(it)
            captured_document.post {

                bitmap = Bitmap.createScaledBitmap(
                    bitmap,
                    captured_document.width,
                    captured_document.height,
                    true
                )
                captured_document.setImageBitmap(bitmap)
            }
        }
    }

}
