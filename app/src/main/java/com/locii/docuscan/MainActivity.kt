package com.locii.docuscan

import android.content.Intent
import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import com.locii.docuscanlib.CameraPreviewActivity
import kotlinx.android.synthetic.main.activity_main.*


class MainActivity : AppCompatActivity() {

    var paths: Array<String?> = emptyArray()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        start_capturing.setOnClickListener {

            val distance = try {

                edit_distance.text.toString().toInt()
            } catch (e: Exception) {
                300
            }
            val sharpness = try {

                edit_sharpness.text.toString().toInt()
            } catch (e: Exception) {
                300
            }

            startActivityForResult(
                CameraPreviewActivity.createGuidedBoxIntent(
                    this,
                    85f / 55f/*ratio of DL*/,
                    distance,
                    sharpness
                ), 126
            )
        }
        view_results.setOnClickListener {
            if (paths.isNotEmpty()) {
                viewResults()
            }
        }

    }

    override fun onResume() {
        super.onResume()
        view_results.visibility = if (paths.isNotEmpty()) View.VISIBLE else View.GONE
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)

        data?.getStringArrayExtra("paths")?.let {

            paths = it
            viewResults()
        }
    }

    private fun viewResults() {
        startActivity(ResultsPreviewActivity.createForPaths(paths, this))
    }

}
