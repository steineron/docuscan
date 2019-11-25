package com.locii.docuscanlib

import org.junit.Before

import org.junit.Assert.*
import org.junit.Test

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