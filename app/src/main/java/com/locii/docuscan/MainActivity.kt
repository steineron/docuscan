package com.locii.docuscan

import android.content.Intent
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.locii.docuscanlib.CameraPreviewActivity
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        start_capturing.setOnClickListener {
            Intent(this, CameraPreviewActivity::class.java).also {

                startActivityForResult(it, 123)
            }
        }

    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)

        data?.data?.let {
            captured_document.setImageURI(it)
        }
    }

}
