
package ossbuild.gstreamer;

import ossbuild.init.SystemLoader;

/**
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
class Loader extends SystemLoader {

	@Override
	public void load() throws Throwable {
		Native.initialize();
	}

	@Override
	public void unload() throws Throwable {
		Native.unload();
	}

}
