package com.locii.docuscan

import android.content.Context
import android.content.Intent
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.os.Bundle
import android.widget.ImageView
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_results.*


class ResultsPreviewActivity : AppCompatActivity() {

    companion object {

        fun createForPaths(paths: Array<String?>, duration:Long, context: Context): Intent =
            Intent(context, ResultsPreviewActivity::class.java)
                .putExtra("paths", paths)
                .putExtra("duration", duration)

    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_results)

        preview_finished.setOnClickListener {
            finish()
        }

        intent?.getStringArrayExtra("paths")?.let { paths ->

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
        intent?.getLongExtra("duration",0)?.let {
            val s = "Scanning completed in ${it / 1000} seconds"
            summary_text.text = s
        }
    }

}
