package ossbuild.extract;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import ossbuild.OS;
import ossbuild.Path;
import ossbuild.Sys;
import ossbuild.extract.processors.FileProcessor;
import ossbuild.extract.processors.LibraryProcessor;
import static org.junit.Assert.*;

/**
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public class ResourcesTest {

	public ResourcesTest() {
	}

	@BeforeClass
	public static void setUpClass() throws Exception {
	}

	@AfterClass
	public static void tearDownClass() throws Exception {
	}

	@Before
	public void setUp() {
	}

	@After
	public void tearDown() {
	}

	@Test
	public void testVariables() {
		String tmpDir = Path.tempDirectory;
		String homeDir = Path.homeDirectory;

		assertEquals("/" + tmpDir + "/tmp/" + tmpDir + "/" + homeDir + "/tmp", Variables.process("/${tmp}/tmp/${tmp}/${home}/tmp"));

		assertEquals("/home/", Variables.process("/home/"));
		assertEquals(tmpDir + "/", Variables.process("${tmp}/"));
		assertEquals("/" + tmpDir + "/", Variables.process("/${tmp}/"));
		assertEquals(homeDir + "/", Variables.process("${home}/"));
		assertEquals("/" + homeDir + "/", Variables.process("/${home}/"));
	}

	@Test
	public void testResourceProcessorFactory() {
		assertFalse(ResourceProcessorFactory.getDefaultProcessors().isEmpty());

		final ResourceProcessorFactory factory = ResourceProcessorFactory.newInstance();
		assertNotNull(factory);

		assertFalse(factory.addProcessors(ResourcesTest.class));
	}

	@Test
	public void testRead() {
		assertTrue("This unit test requires Windows to complete", Sys.isOS(OS.Windows));
		
		Resources r1 = Resources.newInstance(
			Sys.createPlatformPackageResourcePrefix("resources.extraction") + "test.xml"
		);
		assertNotNull(r1);

		assertEquals(1, r1.getTotalPackageCount());
		assertEquals(5, r1.getTotalResourceCount());

		Resources r2 = Resources.newInstance(
			Package.newInstance("test", ".")
		);
		assertEquals(1, r2.getTotalPackageCount());
		assertEquals(0, r2.getTotalResourceCount());

		Resources r3 = Resources.newInstance(
			Package.newInstance("test", "."),
			Package.newInstance("test", ".")
		);
		assertEquals(2, r3.getTotalPackageCount());
		assertEquals(0, r3.getTotalResourceCount());

		Resources r4 = Resources.newInstance(
			Package.newInstance(
				"test",
				".",
				ResourceProcessorFactory.newProcessor(FileProcessor.class),
				ResourceProcessorFactory.newProcessor(FileProcessor.class)
			),
			Package.newInstance(
				"test",
				".",
				ResourceProcessorFactory.newProcessor(LibraryProcessor.class),
				ResourceProcessorFactory.newProcessor(LibraryProcessor.class)
			)
		);
		assertEquals(2, r4.getTotalPackageCount());
		assertEquals(4, r4.getTotalResourceCount());
	}

	@Test
	public void testExtract() throws InterruptedException, ExecutionException {
		assertNotNull(Sys.getPlatformName());

		final Resources r = Resources.newInstance(
			Sys.createPlatformPackageResourceName("resources.extraction", "test.xml")
		);
		assertNotNull(r);

		Future f = r.extract(new ResourceProgressListenerAdapter() {
			@Override
			public void begin(int totalNumberOfResources, int totalNumberOfPackages, long totalNumberOfBytes, long startTime) {
				System.out.println("Extraction beginning...");
			}

			@Override
			public void report(int totalNumberOfResources, int totalNumberOfPackages, long totalNumberOfBytes, long numberOfBytesCompleted, int numberOfResourcesCompleted, int numberOfPackagesCompleted, long startTime, long duration, String message) {
				System.out.println(numberOfPackagesCompleted + "/" + totalNumberOfPackages + " packages completed");
				System.out.println(numberOfResourcesCompleted + "/" + totalNumberOfResources + " resources completed");
				System.out.println(numberOfBytesCompleted + "/" + totalNumberOfBytes + " bytes completed");
				System.out.println();
			}

			@Override
			public void end(boolean success, int totalNumberOfResources, int totalNumberOfPackages, long totalNumberOfBytes, long numberOfBytesCompleted, int numberOfResourcesCompleted, int numberOfPackagesCompleted, long startTime, long endTime) {
				if (success)
					System.out.println("Operation success");
				else
					System.out.println("Operation failed");
				System.out.println(numberOfPackagesCompleted + "/" + totalNumberOfPackages + " packages completed");
				System.out.println(numberOfResourcesCompleted + "/" + totalNumberOfResources + " resources completed");
				System.out.println(numberOfBytesCompleted + "/" + totalNumberOfBytes + " bytes completed");
				System.out.println();
			}

			@Override
			public void error(Throwable exception, String message) {
				assertTrue(false);
			}
		});

		//Wait for operation to complete
		Object o = f.get();
		assertTrue(f.isDone() && !f.isCancelled());

		assertTrue(Path.combine(Path.tempDirectory, "ossbuild/test/test/test.dll").exists());
		assertTrue(Path.combine(Path.tempDirectory, "ossbuild/test/test/test2.txt").exists());
	}
}
