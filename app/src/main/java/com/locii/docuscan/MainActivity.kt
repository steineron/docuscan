package com.locii.docuscan

import android.content.Context
import android.content.Intent
import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import com.locii.docuscanlib.CameraPreviewActivity
import kotlinx.android.synthetic.main.activity_main.*


class MainActivity : AppCompatActivity() {

    var paths: Array<String?> = emptyArray()
    var duration: Long = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        getSharedPreferences("MainActivity", Context.MODE_PRIVATE).run {
            getInt("distance", 300).toString().also {
                edit_distance.setText(it)
            }
            getInt("sharpness", 7).toString().also {
                edit_sharpness.setText(it)
            }
            getInt("minLineLength", 20).toString().also {
                edit_line_length.setText(it)
            }
            getInt("linesThreshold", 25).toString().also {
                edit_threshold.setText(it)
            }
            getInt("maxLineGap", 5).toString().also {
                edit_gap.setText(it)
            }
            getInt("edges", 3).toString().also {
                edit_sides.setText(it)
            }
            getBoolean("dev", true).also {
                dev_images.isChecked = it
            }
        }

        start_capturing.setOnClickListener {

            val distance = try {

                edit_distance.text.toString().toInt()
            } catch (e: Exception) {
                300
            }
            val sharpness = try {

                edit_sharpness.text.toString().toInt()
            } catch (e: Exception) {
                7
            }
            val length = try {

                edit_line_length.text.toString().toInt()
            } catch (e: Exception) {
                20
            }
            val threshold = try {

                edit_threshold.text.toString().toInt()
            } catch (e: Exception) {
                25
            }
            val gap = try {

                edit_gap.text.toString().toInt()
            } catch (e: Exception) {
                5
            }
            val edges = try {
                edit_sides.text.toString().toInt()
            } catch (e: Exception) {
                3
            }
            val gutter = try {
                edit_gutter.text.toString().toInt()
            } catch (e: Exception) {
                50
            }

            getSharedPreferences("MainActivity", Context.MODE_PRIVATE).edit().run {
                putInt("distance", distance)
                putInt("sharpness", sharpness)
                putInt("minLineLength", length)
                putInt("linesThreshold", threshold)
                putInt("maxLineGap", gap)
                putInt("edges", edges)
                putInt("gutter", gutter)
                putBoolean("dev", dev_images.isChecked)
                commit()
            }


            startActivityForResult(
                CameraPreviewActivity.createGuidedBoxIntent(
                    this,
                    85f / 55f/*ratio of DL*/,
                    distance,
                    sharpness,
                    length,
                    threshold,
                    gap,
                    edges,
                    gutter,
                    dev_images.isChecked
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

        data?.apply {
            getStringArrayExtra("paths")?.let {

                paths = it
            }
            getLongExtra("duration", 0).let {
                duration = it
            }
            viewResults()

        }
    }

    private fun viewResults() {
        startActivity(ResultsPreviewActivity.createForPaths(paths, duration, this))
    }

}
