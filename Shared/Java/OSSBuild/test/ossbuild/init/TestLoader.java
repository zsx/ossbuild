
package ossbuild.init;

import static org.junit.Assert.*;

/**
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public class TestLoader extends SystemLoader {

	@Override
	public void load() throws Throwable {
		assertTrue(true);
	}

	@Override
	public void unload() throws Throwable {
		assertTrue(true);
	}

}
