
package ossbuild.extract;

/**
 * Implements {@link IResourceCallback} and provides genericized parameters
 * for convencience.
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public abstract class ResourceCallback<T> implements IResourceCallback {
	//<editor-fold defaultstate="collapsed" desc="Variables">
	private T param;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	public ResourceCallback() {
		this.param = null;
	}

	public ResourceCallback(T Parameter) {
		this.param = Parameter;
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="IResourceCallback Methods">
	public void error(Resources resources) {
		error(resources, param);
	}

	public void prepare(Resources resources) {
		prepare(resources, param);
	}

	public void cancelled(Resources resources) {
		cancelled(resources, param);
	}

	public void completed(Resources resources) {
		completed(resources, param);
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Overrides">
	protected void error(Resources resources, T param) {
	}

	protected void prepare(Resources resources, T param) {
	}

	protected void cancelled(Resources resources, T param) {
	}
	//</editor-fold>
	
	//<editor-fold defaultstate="collapsed" desc="Abstract Methods">
	protected abstract void completed(Resources resources, T param);
	//</editor-fold>
}
