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

// Your IDE likely can auto-import these classes, but there are several
// different implementations so we list them here to disambiguate.

import android.Manifest
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.graphics.Bitmap
import android.graphics.Matrix
import android.graphics.PointF
import android.graphics.RectF
import android.os.Bundle
import android.util.Log
import android.util.Size
import android.view.*
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.camera.core.*
import androidx.camera.core.ImageCapture.OnImageCapturedListener
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.lifecycle.LifecycleOwner
import org.opencv.android.Utils
import org.opencv.core.CvType.CV_8UC4
import org.opencv.core.Mat
import org.opencv.core.Point
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.text.SimpleDateFormat
import java.util.*
import java.util.concurrent.Executors


// This is an arbitrary number we are using to keep track of the permission
// request. Where an app has multiple context for requesting permission,
// this can help differentiate the different contexts.
private const val REQUEST_CODE_PERMISSIONS = 10

// This is an array of all the permission specified in the manifest.
private val REQUIRED_PERMISSIONS = arrayOf(Manifest.permission.CAMERA)

/**
 * CameraPreviewActivity
 *
 * @author steineron (Ron Steiner)
 * @version 1
 */


class CameraPreviewActivity : AppCompatActivity(), LifecycleOwner {

    companion object {
        fun createGuidedBoxIntent(context: Context, ratio: Float): Intent =
            Intent(context, CameraPreviewActivity::class.java).putExtra("ratio", ratio)
    }

    private var guideView: GuideView? = null
    private var documentRatio: Float = 0f

    // optional TL and BR that define the ROI (Region Of Interest) rect
    // e.g. DL has a 85:55 mm ratio so that can be used to draw a ROI to guild the user
    private var topLeft: Point? = null
    private var bottomRight: Point? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.main_camera)

        //make  it full screen
        window.decorView.systemUiVisibility = (
                // Set the content to appear under the system bars so that the
                // content doesn't resize when the system bars hide and show.
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                        or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        // Hide the nav bar and status bar
                        or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        or View.SYSTEM_UI_FLAG_FULLSCREEN)


        //get the Top-Left, Bottom-Right points for the document (if specified)
        intent.getFloatExtra("ratio", 0f).let {

            documentRatio = it

            // for example - the DL is 85:55 mm
            if (it > 0) {
                val display: Display = windowManager.defaultDisplay
                val size = android.graphics.Point()
                display.getSize(size)
                // invert those to create a landscape effect
                val width: Int = size.x
                val height: Int = size.y


                // assume the width should capture 70% of the screen and the height should match:
                calculateGuideTLBR(it, width, height)

                // add the guide view
                guideView?.let { guide ->
                    (guide.parent as? ViewGroup)?.apply {
                        removeView(guide)
                    }
                    guideView = null
                }

                GuideView(this@CameraPreviewActivity).also { guide ->
                    guide.topLeft =
                        PointF(topLeft!!.x.toFloat(), topLeft!!.y.toFloat())
                    guide.bottomRight = PointF(
                        bottomRight!!.x.toFloat(),
                        bottomRight!!.y.toFloat()
                    )

                    val layoutParams = ViewGroup.LayoutParams(
                        ViewGroup.LayoutParams.MATCH_PARENT,
                        ViewGroup.LayoutParams.MATCH_PARENT
                    )
                    addContentView(guide, layoutParams)
                    guideView = guide

                }
            }
        }
        // Add this at the end of onCreate function

        textureView = findViewById(R.id.view_finder)

        // Request camera permissions
        if (allPermissionsGranted()) {
            /*
                Instead of calling `startCamera()` on the main thread, we use `viewFinder.post { ... }`
                to make sure that `viewFinder` has already been inflated into the view when `startCamera()` is called.
             */
            textureView.post { startCamera() }
        } else {
            ActivityCompat.requestPermissions(
                this, REQUIRED_PERMISSIONS, REQUEST_CODE_PERMISSIONS
            )
        }

        // Every time the provided texture view changes, recompute layout
        textureView.addOnLayoutChangeListener { _, _, _, _, _, _, _, _, _ ->
            updateTransform()
        }
    }

    private fun updateGuideView() {
        guideView?.apply {
            topLeft =
                PointF(topLeft!!.x.toFloat(), topLeft!!.y.toFloat())
            bottomRight = PointF(
                bottomRight!!.x.toFloat(),
                bottomRight!!.y.toFloat()
            )
        }
    }

    private fun calculateGuideTLBR(ratio: Float, width: Int, height: Int) {
        val guidWidth = width * .70
        val guideHeight = guidWidth * ratio

        topLeft = Point((width * .15), ((height - guideHeight) / 2))

        bottomRight = Point((topLeft!!.x + guidWidth), (topLeft!!.y + guideHeight))

        Log.d("LOCII", "tl: $topLeft, br:$bottomRight")
    }

    // Add this after onCreate

    private val analysisExecutor = Executors.newSingleThreadExecutor()
    private val processingExecutor = Executors.newSingleThreadExecutor()
    private lateinit var textureView: TextureView

    private fun startCamera() {


        // TO IMPLEMENT IMAGE CAPTURE:
//        https://codelabs.developers.google.com/codelabs/camerax-getting-started/#6


        // Bind use cases to lifecycle
        // If Android Studio complains about "this" being not a LifecycleOwner
        // try rebuilding the project or updating the appcompat dependency to
        // version 1.1.0 or higher.
        CameraX.bindToLifecycle(
            this,
            createPreview(),
            createImageAnalysis() /*, createImageCapture()*/
        )
    }

    /**
     * Process result from permission request dialog box, has the request
     * been granted? If yes, start Camera. Otherwise display a toast
     */
    override fun onRequestPermissionsResult(
        requestCode: Int, permissions: Array<String>, grantResults: IntArray
    ) {
        if (requestCode == REQUEST_CODE_PERMISSIONS) {
            if (allPermissionsGranted()) {
                textureView.post { startCamera() }
            } else {
                Toast.makeText(
                    this,
                    "Permissions not granted by the user.",
                    Toast.LENGTH_SHORT
                ).show()
                finish()
            }
        }
    }

    /**
     * Check if all permission specified in the manifest have been granted
     */
    private fun allPermissionsGranted() = REQUIRED_PERMISSIONS.all {
        ContextCompat.checkSelfPermission(
            baseContext, it
        ) == PackageManager.PERMISSION_GRANTED
    }


    private fun createPreview(): Preview {

        val screen =
            Size(textureView.width, textureView.height) //size of the screen


        val previewConfig =
            PreviewConfig.Builder().setTargetResolution(screen)
                .build()
        val preview = Preview(previewConfig)

        preview.setOnPreviewOutputUpdateListener { output ->
            val parent = textureView.parent as ViewGroup
            parent.removeView(textureView)
            parent.addView(textureView, 0)
            textureView.surfaceTexture = output.surfaceTexture
            updateTransform()
        }
        return preview
    }


    private fun createImageAnalysis(): ImageAnalysis? { // Setup image analysis pipeline that computes average pixel luminance

        val imageAnalysisConfig: ImageAnalysisConfig = ImageAnalysisConfig.Builder()
            .setImageReaderMode(ImageAnalysis.ImageReaderMode.ACQUIRE_LATEST_IMAGE)
            .setBackgroundExecutor(analysisExecutor)
            /*.setImageQueueDepth(1)*/.build()
        val imageAnalysis = ImageAnalysis(imageAnalysisConfig)
        imageAnalysis.setAnalyzer(analysisExecutor, object : ImageAnalysis.Analyzer {

            var processFrames = true
            val paths = mutableListOf<String>()

            val nativeDocScanner = object : DocuScan() {

                override fun onIntermitentMat(matAddrOut: Long) {
                    Log.d("LOCII", "onIntermitentMat: mat address: $matAddrOut")

                    var temp = Mat(matAddrOut)
                    temp.clone().also {
                        processingExecutor.execute {
                            Log.d(
                                "LOCII",
                                "onIntermitentMat: saving Mat at address: ${it.nativeObjAddr}"
                            )

                            val (path, success) = saveAsBitmap(it)
                            if (success) {
                                paths.add(path)
                            }
                            it.release()
                        }

                    }
                }

                override fun onResultMat(matAddrOut: Long) {
                    Log.d("LOCII", "onResultMat: mat address: $matAddrOut")
                    processFrames = false

                    var temp = Mat(matAddrOut)
                    temp.clone().also {
                        processingExecutor.execute {
                            Log.d(
                                "LOCII",
                                "onResultMat: saving Mat at address: ${it.nativeObjAddr}"
                            )
                            val (path, success) = saveAsBitmap(it)
                            if (success) {

                                paths.add(path)
                                val data = Intent().putExtra("paths", paths.toTypedArray())
                                setResult(RESULT_OK, data)
//                                result.release()
                                finish()
                            }
                            it.release()
                        }
                    }
                }

            }


            /**
             * Analyzes an image to produce a result.
             *
             *
             * This method is called once for each image from the camera, and called at the
             * frame rate of the camera.  Each analyze call is executed sequentially.
             *
             *
             * The caller is responsible for ensuring this analysis method can be executed quickly
             * enough to prevent stalls in the image acquisition pipeline. Otherwise, newly available
             * images will not be acquired and analyzed.
             *
             *
             * The image passed to this method becomes invalid after this method returns. The caller
             * should not store external references to this image, as these references will become
             * invalid.
             *
             *
             * Processing should complete within a single frame time of latency, or the image data
             * should be copied out for longer processing.  Applications can be skip analyzing a frame
             * by having the analyzer return immediately.
             *
             * @param image           The image to analyze
             * @param rotationDegrees The rotation which if applied to the image would make it match
             * the current target rotation of [ImageAnalysis], expressed in
             * degrees in the range `[0..360)`.
             */
            override fun analyze(image: ImageProxy?, rotationDegrees: Int) {
                if (!processFrames) {
                    return
                }
                val bitmap = textureView.bitmap ?: return
                val mat = Mat()
                val temp1 = Mat()
                val temp2 = Mat()
                Utils.bitmapToMat(bitmap, mat)
                mat.copyTo(temp1)
                mat.copyTo(temp2)

                nativeDocScanner.distance = 30000
                nativeDocScanner.sharpness = 4
                nativeDocScanner.guidingRect = RectF(
                    topLeft?.x?.toFloat() ?: 0.0f,
                    topLeft?.y?.toFloat() ?: 0.0f,
                    bottomRight?.x?.toFloat() ?: 0.0f,
                    bottomRight?.y?.toFloat() ?: 0.0f
                )

                nativeDocScanner.scan(
                    mat.nativeObjAddr,
                    temp1.nativeObjAddr,
                    temp2.nativeObjAddr
                )
            }
        })

        return imageAnalysis
    }

    private fun saveAsBitmap(mat: Mat): Pair<String, Boolean> {

        val dateFormat = SimpleDateFormat("yyyy-MM-dd-HH-mm-ss.SSS")
        mat.convertTo(mat, CV_8UC4)

        val path =
            filesDir.toString() + File.separator + dateFormat.format(Date()) + "-result.png"

        //  write the bitmap to file - read it afterwords
        var success = false
        val bmp =
            Bitmap.createBitmap(mat.cols(), mat.rows(), Bitmap.Config.ARGB_8888)
        Utils.matToBitmap(mat, bmp)
        try {
            val out = FileOutputStream(path)
            bmp.compress(
                Bitmap.CompressFormat.PNG,
                100,
                out
            )
            success = true
        } catch (e: IOException) {
            e.printStackTrace()
        }
        bmp.recycle()
        mat.release()
        return Pair(path, success)
    }

    private fun createImageCapture(): ImageCapture? {
        val imageCaptureConfig =
            ImageCaptureConfig.Builder().setCaptureMode(ImageCapture.CaptureMode.MIN_LATENCY)
                .setTargetRotation(windowManager.defaultDisplay.rotation).build()
        val imgCapture = ImageCapture(imageCaptureConfig)

        imgCapture.takePicture(
            Executors.newSingleThreadExecutor(),
            object : OnImageCapturedListener() {
                override fun onCaptureSuccess(
                    image: ImageProxy,
                    rotationDegrees: Int
                ) {
                    val bitmap: Bitmap = textureView.bitmap
//                        showAcceptedRejectedButton(true)
//                        ivBitmap.setImageBitmap(bitmap)
                }

                override fun onError(
                    imageCaptureError: ImageCapture.ImageCaptureError,
                    message: String,
                    cause: Throwable?
                ) {
                    super.onError(imageCaptureError, message, cause)
                }
            })
        /*File file = new File(
                Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES), "" + System.currentTimeMillis() + "_JDCameraX.jpg");
        imgCapture.takePicture(file, new ImageCapture.OnImageSavedListener() {
            @Override
            public void onImageSaved(@NonNull File file) {
                Bitmap bitmap = textureView.getBitmap();
                showAcceptedRejectedButton(true);
                ivBitmap.setImageBitmap(bitmap);
            }

            @Override
            public void onError(@NonNull ImageCapture.UseCaseError useCaseError, @NonNull String message, @Nullable Throwable cause) {

            }
        });*/

        return imgCapture
    }

    private fun updateTransform() {
        val matrix = Matrix()

        // Compute the center of the view finder
        val centerX = textureView.width / 2f
        val centerY = textureView.height / 2f

        // Correct preview output to account for display rotation
        val rotationDegrees = when (textureView.display.rotation) {
            Surface.ROTATION_0 -> 0
            Surface.ROTATION_90 -> 90
            Surface.ROTATION_180 -> 180
            Surface.ROTATION_270 -> 270
            else -> return
        }
        Log.d("LOCII", "rotation: $rotationDegrees")

        matrix.postRotate(-rotationDegrees.toFloat(), centerX, centerY)

        // Finally, apply transformations to our TextureView
        textureView.setTransform(matrix)

        // update hte TL BR coordinates
        if (documentRatio > 0) {

            calculateGuideTLBR(documentRatio, textureView.width, textureView.height)

            when (textureView.display.rotation) {
                Surface.ROTATION_180, Surface.ROTATION_270 -> {
                    val tl = topLeft
                    val br = bottomRight
                    topLeft = br
                    bottomRight = tl
                }
                else -> {
                }
            }
            updateGuideView()
        }
    }


//    private fun showAcceptedRejectedButton(acceptedRejected: Boolean) {
//        if (acceptedRejected) {
//            CameraX.unbind(preview, imageAnalysis)
//            llBottom.setVisibility(View.VISIBLE)
//            btnCapture.hide()
//            textureView.visibility = View.GONE
//        } else {
//            btnCapture.show()
//            llBottom.setVisibility(View.GONE)
//            textureView.visibility = View.VISIBLE
//            textureView.post { startCamera() }
//        }
//    }
}