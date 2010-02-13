package ossbuild.extract;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import ossbuild.Path;
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
		Resources r1 = Resources.newInstance("/resources/extraction/test.xml");
		assertNotNull(r1);

		assertEquals(1, r1.getTotalPackageCount());
		assertEquals(2, r1.getTotalResourceCount());

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
}
