
package ossbuild.gstreamer;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import ossbuild.OS;
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
		final String os_gpl = Sys.createPlatformPackageResourcePrefix("resources.gstreamer.gpl", Sys.getOS());
		final String osfamily_gpl = Sys.createPlatformPackageResourcePrefix("resources.gstreamer.gpl", Sys.getOSFamily());

		final String os_lgpl = Sys.createPlatformPackageResourcePrefix("resources.gstreamer.lgpl", Sys.getOS());
		final String osfamily_lgpl = Sys.createPlatformPackageResourcePrefix("resources.gstreamer.lgpl", Sys.getOSFamily());

		RESOURCE_DEFINITION_PREFIX = (
			Sys.isResourceAvailable(os_gpl + "resources.xml") ? os_gpl :
				Sys.isResourceAvailable(osfamily_gpl + "resources.xml") ? osfamily_gpl :
					Sys.isResourceAvailable(os_lgpl + "resources.xml") ? os_lgpl :
						Sys.isResourceAvailable(osfamily_lgpl + "resources.xml") ? osfamily_lgpl :
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

	//<editor-fold defaultstate="collapsed" desc="Public Static Methods">
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
	//</editor-fold>
}
