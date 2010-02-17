
package ossbuild.extract;

/**
 * Implements {@link IResourceCallback} and provides genericized parameters
 * for convencience.
 *
 * @author David Hoyt <dhoyt@hoytsoft.org>
 */
public abstract class ResourceCallbackChain<T> implements IResourceCallback {
	//<editor-fold defaultstate="collapsed" desc="Variables">
	private T param;
	private IResourceCallback chainedCallback;
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Initialization">
	public ResourceCallbackChain(final IResourceCallback ChainedCallback) {
		init(ChainedCallback, null);
	}

	public ResourceCallbackChain(final IResourceCallback ChainedCallback, T Parameter) {
		init(ChainedCallback, Parameter);
	}

	private void init(final IResourceCallback ChainedCallback, final T Parameter) {
		this.param = Parameter;
		this.chainedCallback = ChainedCallback;
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="Getters">
	public T getParameter() {
		return param;
	}
	
	public IResourceCallback getChainedCallback() {
		return chainedCallback;
	}
	//</editor-fold>

	//<editor-fold defaultstate="collapsed" desc="IResourceCallback Methods">
	@Override
	public void error(Resources resources) {
		error(resources, param);
		if (chainedCallback != null)
			chainedCallback.error(resources);
	}

	@Override
	public void prepare(Resources resources) {
		prepare(resources, param);
		if (chainedCallback != null)
			chainedCallback.prepare(resources);
	}

	@Override
	public void cancelled(Resources resources) {
		cancelled(resources, param);
		if (chainedCallback != null)
			chainedCallback.cancelled(resources);
	}

	@Override
	public void completed(Resources resources) {
		completed(resources, param);
		if (chainedCallback != null)
			chainedCallback.completed(resources);
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
