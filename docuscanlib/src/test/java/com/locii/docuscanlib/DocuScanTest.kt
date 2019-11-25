package com.locii.docuscanlib

import org.junit.Test

import org.junit.Assert.*

class DocuScanTest {

    @Test
    fun stringFromJNI() {

        DocuScan().stringFromJNI().also {
            assertNotNull(it)
            assertEquals("hello",it)
        }
    }
}