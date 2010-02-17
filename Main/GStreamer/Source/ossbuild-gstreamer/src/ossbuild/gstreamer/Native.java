
package ossbuild.gstreamer;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import ossbuild.StringUtil;
import ossbuild.Sys;
import ossbuild.extract.IResourceCallback;
import ossbuild.extract.IResourceProgressListener;
import ossbuild.extract.MissingResourceException;
import ossbuild.extract.ResourceCallbackChain;
import ossbuild.extract.Resources;

/**
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public class Native {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final String
		  RESOURCE_DEFINITION_PREFIX
	;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Variables">
	private static final Object lock = new Object();

	private static boolean initialized = false;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	static {
		//Determine the prefix to use based off of what's available
		//Check if GPL is available first.
		final String gpl = Sys.createPlatformPackageResourcePrefix("resources.gstreamer.gpl");
		final String lgpl = Sys.createPlatformPackageResourcePrefix("resources.gstreamer.lgpl");

		RESOURCE_DEFINITION_PREFIX = (
			Sys.isResourceAvailable(gpl + "resources.xml") ? gpl :
				Sys.isResourceAvailable(lgpl + "resources.xml") ? lgpl :
					StringUtil.empty
		);

		if (StringUtil.isNullOrEmpty(RESOURCE_DEFINITION_PREFIX))
			throw new MissingResourceException("Unable to locate the main resource extraction file");
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Getters">
	public static Object getLock() {
		return lock;
	}
	//</editor-fold>

	public static Resources createResourceExtractor() {
		return Resources.newInstance(RESOURCE_DEFINITION_PREFIX + "resources.xml");
	}

	public static void initialize() {
		synchronized(lock) {
			try {
				//Blocks waiting on the resources to be extracted in a separate thread.
				//This is the behavior you'd normally want.
				final Future f = initialize(IResourceProgressListener.None, IResourceCallback.None);
				if (f != null)
					f.get();
			} catch(InterruptedException ie) {
			} catch(ExecutionException ee) {
			}
		}
	}

	public static Future initialize(final IResourceProgressListener progress, final IResourceCallback callback) {
		synchronized(lock) {
			if (initialized)
				return null;

			initialized = true;

			//Extracts on a separate thread - this allows you to display the progress as
			//the libraries are extracted and loaded.
			final Resources resources = createResourceExtractor();
			return resources.extract(progress, new ResourceCallbackChain(callback) {
				@Override
				protected void completed(final Resources res, final Object param) {
					System.setProperty("apple.awt.graphics.UseQuartz", "false");
					org.gstreamer.Gst.init(
						"OSSBuild Application",
						new String[] {
						}
					);
				}
			});
		}
	}

	public static void unload() {
		synchronized(lock) {
			if (!initialized)
				return;
		}
	}

}
