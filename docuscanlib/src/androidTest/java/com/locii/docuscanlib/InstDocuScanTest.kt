package com.locii.docuscanlib

import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.Before

import org.junit.Assert.*
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class InstDocuScanTest {

    lateinit var docuScan: DocuScan
    @Before
    fun setUp() {
        docuScan = DocuScan()
    }

    @Test
    fun that_the_import_works() {
        // setup
        // (none)

        // execute

        // verify
        assertEquals(docuScan.stringFromJNI(), "Hello from C++")
    }
}