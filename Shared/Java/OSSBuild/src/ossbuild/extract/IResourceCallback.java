
package ossbuild.extract;

/**
 * Called when extraction has completed. Done on the same worker thread
 * used for loading.
 * 
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public interface IResourceCallback<T> {
	//<editor-fold defaultstate="collapsed" desc="Constants">
	public static final IResourceCallback None = null;
	//</editor-fold>

	void error(Resources resources);
	void prepare(Resources resources);
	void cancelled(Resources resources);
	void completed(Resources resources);
}
