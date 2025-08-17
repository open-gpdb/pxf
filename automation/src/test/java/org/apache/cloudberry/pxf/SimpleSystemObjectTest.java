package org.apache.cloudberry.pxf;

import jsystem.framework.system.SystemManagerImpl;
import org.greenplum.pxf.automation.components.pxf.Pxf;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

/**
 * Simple test class to verify system object initialization without inheriting from BaseTestParent.
 * This helps identify which system objects can be initialized independently.
 */
public class SimpleSystemObjectTest {

    private SystemManagerImpl systemManager;

    @BeforeClass
    public void setUp() {
        systemManager = SystemManagerImpl.getInstance();
    }

    @Test(description = "Test cluster system object initialization")
    public void testClusterInit() throws Exception {
        System.out.println("Testing cluster system object...");
        try {
            Object cluster = systemManager.getSystemObject("cluster");
            System.out.println("✓ Cluster system object initialized successfully");
            System.out.println("  Class: " + cluster.getClass().getName());
        } catch (Exception e) {
            System.out.println("✗ Cluster system object initialization failed: " + e.getMessage());
            throw e;
        }
    }

    @Test(description = "Test GPDB system object initialization")
    public void testGpdbInit() throws Exception {
        System.out.println("Testing GPDB system object...");
        try {
            Object gpdb = systemManager.getSystemObject("gpdb");
            System.out.println("✓ GPDB system object initialized successfully");
            System.out.println("  Class: " + gpdb.getClass().getName());
        } catch (Exception e) {
            System.out.println("✗ GPDB system object initialization failed: " + e.getMessage());
            throw e;
        }
    }

    @Test(description = "Test GPDB2 system object initialization")
    public void testGpdb2Init() throws Exception {
        System.out.println("Testing GPDB2 system object...");
        try {
            Object gpdb2 = systemManager.getSystemObject("gpdb2");
            System.out.println("✓ GPDB2 system object initialized successfully");
            System.out.println("  Class: " + gpdb2.getClass().getName());
        } catch (Exception e) {
            System.out.println("✗ GPDB2 system object initialization failed: " + e.getMessage());
            throw e;
        }
    }

    @Test(description = "Test HDFS system object initialization")
    public void testHdfsInit() throws Exception {
        System.out.println("Testing HDFS system object...");
        try {
            Object hdfs = systemManager.getSystemObject("hdfs");
            System.out.println("✓ HDFS system object initialized successfully");
            System.out.println("  Class: " + hdfs.getClass().getName());
        } catch (Exception e) {
            System.out.println("✗ HDFS system object initialization failed: " + e.getMessage());
            throw e;
        }
    }

    @Test(description = "Test file system object initialization")
    public void testFileInit() throws Exception {
        System.out.println("Testing file system object...");
        try {
            Object fileSystem = systemManager.getSystemObject("file");
            System.out.println("✓ File system object initialized successfully");
            System.out.println("  Class: " + fileSystem.getClass().getName());
        } catch (Exception e) {
            System.out.println("✗ File system object initialization failed: " + e.getMessage());
            throw e;
        }
    }

    @Test(description = "Test S3 system object initialization")
    public void testS3Init() throws Exception {
        System.out.println("Testing S3 system object...");
        try {
            Object s3System = systemManager.getSystemObject("s3");
            System.out.println("✓ S3 system object initialized successfully");
            System.out.println("  Class: " + s3System.getClass().getName());
        } catch (Exception e) {
            System.out.println("✗ S3 system object initialization failed: " + e.getMessage());
            throw e;
        }
    }

    @Test(description = "Test ABFSS system object initialization")
    public void testAbfssInit() throws Exception {
        System.out.println("Testing ABFSS system object...");
        try {
            Object abfssSystem = systemManager.getSystemObject("abfss");
            System.out.println("✓ ABFSS system object initialized successfully");
            System.out.println("  Class: " + abfssSystem.getClass().getName());
        } catch (Exception e) {
            System.out.println("✗ ABFSS system object initialization failed: " + e.getMessage());
            throw e;
        }
    }

    @Test(description = "Test GS system object initialization")
    public void testGsInit() throws Exception {
        System.out.println("Testing GS system object...");
        try {
            Object gsSystem = systemManager.getSystemObject("gs");
            System.out.println("✓ GS system object initialized successfully");
            System.out.println("  Class: " + gsSystem.getClass().getName());
        } catch (Exception e) {
            System.out.println("✗ GS system object initialization failed: " + e.getMessage());
            throw e;
        }
    }

    @Test(description = "Test WASBS system object initialization")
    public void testWasbsInit() throws Exception {
        System.out.println("Testing WASBS system object...");
        try {
            Object wasbsSystem = systemManager.getSystemObject("wasbs");
            System.out.println("✓ WASBS system object initialized successfully");
            System.out.println("  Class: " + wasbsSystem.getClass().getName());
        } catch (Exception e) {
            System.out.println("✗ WASBS system object initialization failed: " + e.getMessage());
            throw e;
        }
    }

    @Test(description = "Test HBase system object initialization")
    public void testHBaseInit() throws Exception {
        System.out.println("Testing HBase system object...");
        try {
            Object hbaseSystem = systemManager.getSystemObject("hbase");
            System.out.println("✓ HBase system object initialized successfully");
            System.out.println("  Class: " + hbaseSystem.getClass().getName());
        } catch (Exception e) {
            System.out.println("✗ HBase system object initialization failed: " + e.getMessage());
            throw e;
        }
    }

    @Test(description = "Test Hive system object initialization")
    public void testHiveInit() throws Exception {
        System.out.println("Testing Hive system object...");
        try {
            Object hiveSystem = systemManager.getSystemObject("hive");
            System.out.println("✓ Hive system object initialized successfully");
            System.out.println("  Class: " + hiveSystem.getClass().getName());
        } catch (Exception e) {
            System.out.println("✗ Hive system object initialization failed: " + e.getMessage());
            throw e;
        }
    }

    @Test(description = "Test Regress system object initialization")
    public void testRegressInit() throws Exception {
        System.out.println("Testing Regress system object...");
        try {
            Object regressSystem = systemManager.getSystemObject("regress");
            System.out.println("✓ Regress system object initialized successfully");
            System.out.println("  Class: " + regressSystem.getClass().getName());
        } catch (Exception e) {
            System.out.println("✗ Regress system object initialization failed: " + e.getMessage());
            throw e;
        }
    }

    @Test(description = "Test PXF system object initialization")
    public void testPxfInit() throws Exception {
        System.out.println("Testing PXF system object...");
        try {
            Pxf pxfSystem = (Pxf) systemManager.getSystemObject("pxf");
            System.out.println("✓ PXF system object initialized successfully");
            System.out.println("  Class: " + pxfSystem.getClass().getName());
        } catch (Exception e) {
            System.out.println("✗ PXF system object initialization failed: " + e.getMessage());
            throw e;
        }
    }

}
